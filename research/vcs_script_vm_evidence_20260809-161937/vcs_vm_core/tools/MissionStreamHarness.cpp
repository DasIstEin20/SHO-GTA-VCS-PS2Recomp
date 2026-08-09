#include "vcs_vm/ScriptVmCore.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {

constexpr std::array<std::uint8_t, 8> kMainEntryMarker{
    0x38, 0x02, 0x0A, 'M', 'A', 'I', 'N', 0x00,
};
constexpr std::size_t kPortableLocalSlots = 106;

struct MissionMetadata {
    std::uint32_t variable_space_size = 0;
    std::uint32_t object_size = 0;
    std::uint16_t num_true_globals = 0;
    std::uint16_t most_globals = 0;
    std::uint32_t largest_mission_script_size = 0;
    std::int16_t mission_script_count = 0;
    std::uint16_t exclusive_mission_count = 0;
    std::vector<std::uint32_t> offsets;
};

struct RunOutcome {
    vcs::vm::ProcessResult result{};
    std::uint32_t last_ip_before = 0;
};

const char* fault_name(vcs::vm::VmFault fault) {
    using vcs::vm::VmFault;
    switch (fault) {
    case VmFault::None: return "None";
    case VmFault::InstructionBudgetExhausted: return "InstructionBudgetExhausted";
    case VmFault::UnsupportedOpcode: return "UnsupportedOpcode";
    case VmFault::TruncatedBytecode: return "TruncatedBytecode";
    case VmFault::Opcode0289RequiresMissionStreamingSubsystem: return "Opcode0289RequiresMissionStreamingSubsystem";
    case VmFault::Opcode013FRequiresPickupModelObjectResolver: return "Opcode013FRequiresPickupModelObjectResolver";
    case VmFault::Opcode013FRequiresPickupGroundHeightProbe: return "Opcode013FRequiresPickupGroundHeightProbe";
    case VmFault::Opcode013FRequiresPickupCreateAdapter: return "Opcode013FRequiresPickupCreateAdapter";
    case VmFault::Opcode01E7RequiresUniqueJumpsTotalAdapter: return "Opcode01E7RequiresUniqueJumpsTotalAdapter";
    case VmFault::Opcode0045RequiresConditionProbe: return "Opcode0045RequiresConditionProbe";
    case VmFault::Opcode00C9RequiresFadeStartAdapter: return "Opcode00C9RequiresFadeStartAdapter";
    case VmFault::Opcode0203RequiresDisplayTextAdapter: return "Opcode0203RequiresDisplayTextAdapter";
    default: return "OtherVmFault";
    }
}

const char* status_name(vcs::vm::NativeHandlerStatus status) {
    return status == vcs::vm::NativeHandlerStatus::Continue ? "CONTINUE" : "YIELD";
}

bool read_u16_le(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t& value) {
    if (offset > bytes.size() || bytes.size() - offset < 2)
        return false;
    value = static_cast<std::uint16_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
    return true;
}

bool read_i16_le(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::int16_t& value) {
    std::uint16_t raw = 0;
    if (!read_u16_le(bytes, offset, raw))
        return false;
    value = static_cast<std::int16_t>(raw);
    return true;
}

bool read_u32_le(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t& value) {
    if (offset > bytes.size() || bytes.size() - offset < 4)
        return false;
    value = static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
    return true;
}

bool parse_positive_size(const char* text, std::size_t maximum, std::size_t& out) {
    try {
        const std::string value(text);
        std::size_t parsed = 0;
        const unsigned long long number = std::stoull(value, &parsed, 10);
        if (parsed != value.size() || number == 0 || number > maximum)
            return false;
        out = static_cast<std::size_t>(number);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_nonnegative_i32(const char* text, std::int32_t& out) {
    try {
        const std::string value(text);
        std::size_t parsed = 0;
        const long long number = std::stoll(value, &parsed, 10);
        if (parsed != value.size() || number < 0 || number > std::numeric_limits<std::int32_t>::max())
            return false;
        out = static_cast<std::int32_t>(number);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_mission_metadata(const std::vector<std::uint8_t>& script_space,
                            MissionMetadata& metadata, std::string& error) {
    // Exact VCS PSP shape confirmed at 0006041C:
    // GetSizeOfVariableSpace() -> +3 -> Read4Bytes -> +8 -> fields/table.
    if (!read_u32_le(script_space, 3, metadata.variable_space_size)) {
        error = "could not read ScriptSpace+3 variable-space size";
        return false;
    }
    if (metadata.variable_space_size > script_space.size() - 3) {
        error = "variable-space size exceeds ScriptSpace";
        return false;
    }
    const std::size_t object_size_address = static_cast<std::size_t>(metadata.variable_space_size) + 3;
    if (!read_u32_le(script_space, object_size_address, metadata.object_size)) {
        error = "could not read object size";
        return false;
    }
    if (metadata.object_size > script_space.size() - 8) {
        error = "object size exceeds ScriptSpace";
        return false;
    }
    const std::size_t fields = static_cast<std::size_t>(metadata.object_size) + 8;
    if (!read_u16_le(script_space, fields + 0x00, metadata.num_true_globals) ||
        !read_u16_le(script_space, fields + 0x02, metadata.most_globals) ||
        !read_u32_le(script_space, fields + 0x04, metadata.largest_mission_script_size) ||
        !read_i16_le(script_space, fields + 0x08, metadata.mission_script_count) ||
        !read_u16_le(script_space, fields + 0x0A, metadata.exclusive_mission_count)) {
        error = "could not read mission metadata fields";
        return false;
    }
    if (metadata.mission_script_count <= 0) {
        error = "mission offset table has no positive entry count";
        return false;
    }
    const std::size_t count = static_cast<std::size_t>(metadata.mission_script_count);
    const std::size_t table = fields + 0x0C;
    if (table > script_space.size() || count > (script_space.size() - table) / 4) {
        error = "mission offset table exceeds ScriptSpace";
        return false;
    }
    metadata.offsets.resize(count);
    for (std::size_t index = 0; index < count; ++index) {
        if (!read_u32_le(script_space, table + index * 4, metadata.offsets[index])) {
            error = "could not read mission offset table entry";
            return false;
        }
    }
    return true;
}

void install_bootstrap_recording_adapters(vcs::vm::ScriptVm& vm) {
    // Deliberately narrow stubs: these are only the already-proved adapters
    // required by the known MAIN bootstrap route, not a general game host.
    vm.set_clock([] { return 0U; });
    vm.set_fade_start_adapter([](float, std::int16_t) {});
    vm.set_pause_menu_toggle_adapter([](bool) {});
    vm.set_display_text_adapter([](std::int32_t, std::int32_t, std::int32_t) {});
}

void write_trace(std::ofstream* trace, const char* phase, std::size_t index,
                 std::uint32_t ip_before, const RunOutcome& outcome) {
    if (trace == nullptr)
        return;
    std::uint32_t probe_ip = ip_before;
    std::int16_t raw_signed = 0;
    const bool raw_present = false; // Raw payload is intentionally not exported by this evidence harness.
    (void)probe_ip;
    (void)raw_signed;
    *trace << "{\"phase\":\"" << phase << "\",\"index\":" << index
           << ",\"ip_before\":\"0x" << std::hex << std::uppercase << ip_before << std::dec
           << "\",\"opcode\":\"0x" << std::hex << std::uppercase << outcome.result.last_opcode << std::dec
           << "\",\"ip_after\":\"0x" << std::hex << std::uppercase << outcome.last_ip_before << std::dec
           << "\",\"status\":\"" << status_name(outcome.result.status)
           << "\",\"fault\":\"" << fault_name(outcome.result.fault) << "\"}" << '\n';
    (void)raw_present;
}

RunOutcome run_until_stop(vcs::vm::ScriptVm& vm, vcs::vm::ScriptThread& thread,
                          std::size_t budget, const char* phase, std::ofstream* trace) {
    RunOutcome outcome{};
    for (std::size_t index = 0; index < budget; ++index) {
        const std::uint32_t ip_before = thread.ip;
        outcome.last_ip_before = ip_before;
        outcome.result = vm.process_one_command(thread);
        outcome.result.executed = index + 1;
        if (trace != nullptr) {
            // Save the post-command IP as a local copy without retaining bytecode.
            RunOutcome trace_outcome = outcome;
            trace_outcome.last_ip_before = thread.ip;
            write_trace(trace, phase, index, ip_before, trace_outcome);
        }
        if (outcome.result.fault != vcs::vm::VmFault::None ||
            outcome.result.status != vcs::vm::NativeHandlerStatus::Continue)
            return outcome;
    }
    outcome.result.status = vcs::vm::NativeHandlerStatus::Yield;
    outcome.result.fault = vcs::vm::VmFault::InstructionBudgetExhausted;
    outcome.result.executed = budget;
    return outcome;
}

void print_result(const char* label, const RunOutcome& outcome, const vcs::vm::ScriptThread& thread) {
    std::cout << label << " executed=" << outcome.result.executed
              << " last_ip=0x" << std::hex << std::uppercase << outcome.last_ip_before
              << " next_ip=0x" << thread.ip
              << " opcode=0x" << std::setw(4) << std::setfill('0') << outcome.result.last_opcode
              << std::dec << std::setfill(' ')
              << " status=" << status_name(outcome.result.status)
              << " fault=" << fault_name(outcome.result.fault) << '\n';
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: vcs_vm_mission_stream_harness <MAIN.SCM> [--parent-commands N] [--mission-commands N] [--resume-parent-commands N] [--mission-mode N] [--mission-selector N] [--synthetic-pickup-dry-run] [--trace-jsonl PATH]\n";
        return 2;
    }

    std::size_t parent_budget = 512;
    std::size_t mission_budget = 512;
    std::size_t parent_resume_budget = 128;
    std::int32_t host_mission_mode = 0;
    std::optional<std::int32_t> mission_selector_override;
    bool synthetic_pickup_dry_run = false;
    std::optional<std::string> trace_path;
    for (int index = 2; index < argc;) {
        const std::string option(argv[index++]);
        if (option == "--parent-commands" && index < argc) {
            if (!parse_positive_size(argv[index++], 100000, parent_budget)) {
                std::cerr << "--parent-commands must be in 1..100000\n";
                return 2;
            }
        } else if (option == "--mission-commands" && index < argc) {
            if (!parse_positive_size(argv[index++], 100000, mission_budget)) {
                std::cerr << "--mission-commands must be in 1..100000\n";
                return 2;
            }
        } else if (option == "--resume-parent-commands" && index < argc) {
            if (!parse_positive_size(argv[index++], 100000, parent_resume_budget)) {
                std::cerr << "--resume-parent-commands must be in 1..100000\n";
                return 2;
            }
        } else if (option == "--mission-mode" && index < argc) {
            if (!parse_nonnegative_i32(argv[index++], host_mission_mode)) {
                std::cerr << "--mission-mode must be a non-negative signed 32-bit integer\n";
                return 2;
            }
        } else if (option == "--mission-selector" && index < argc) {
            std::int32_t parsed_selector = 0;
            if (!parse_nonnegative_i32(argv[index++], parsed_selector)) {
                std::cerr << "--mission-selector must be a non-negative signed 32-bit integer\n";
                return 2;
            }
            mission_selector_override = parsed_selector;
        } else if (option == "--synthetic-pickup-dry-run") {
            synthetic_pickup_dry_run = true;
        } else if (option == "--trace-jsonl" && index < argc) {
            trace_path = argv[index++];
        } else {
            std::cerr << "Usage: vcs_vm_mission_stream_harness <MAIN.SCM> [--parent-commands N] [--mission-commands N] [--resume-parent-commands N] [--mission-mode N] [--mission-selector N] [--synthetic-pickup-dry-run] [--trace-jsonl PATH]\n";
            return 2;
        }
    }

    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Cannot open MAIN.SCM read-only: " << argv[1] << '\n';
        return 2;
    }
    const std::vector<std::uint8_t> file_bytes{
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (file_bytes.size() < 8) {
        std::cerr << "MAIN.SCM is shorter than its VCS header\n";
        return 2;
    }

    std::uint32_t main_script_size = 0;
    std::uint32_t header_largest_mission_size = 0;
    if (!read_u32_le(file_bytes, 0, main_script_size) || !read_u32_le(file_bytes, 4, header_largest_mission_size) ||
        main_script_size > file_bytes.size() - 8) {
        std::cerr << "Invalid VCS SCM header\n";
        return 2;
    }
    const std::vector<std::uint8_t> source_body(file_bytes.begin() + 8, file_bytes.end());

    MissionMetadata metadata;
    std::string metadata_error;
    if (!parse_mission_metadata(source_body, metadata, metadata_error)) {
        std::cerr << "Could not parse confirmed mission metadata: " << metadata_error << '\n';
        return 2;
    }
    if (metadata.largest_mission_script_size != header_largest_mission_size ||
        metadata.largest_mission_script_size == 0 ||
        main_script_size > std::numeric_limits<std::uint32_t>::max() - metadata.largest_mission_script_size) {
        std::cerr << "MAIN.SCM header and parsed mission metadata disagree\n";
        return 2;
    }

    const auto entry_end = source_body.begin() + static_cast<std::ptrdiff_t>(main_script_size);
    const auto marker = std::search(source_body.begin(), entry_end, kMainEntryMarker.begin(), kMainEntryMarker.end());
    if (marker == entry_end || std::search(marker + 1, entry_end, kMainEntryMarker.begin(), kMainEntryMarker.end()) != entry_end) {
        std::cerr << "SCRIPT_NAME MAIN entry marker is missing or non-unique\n";
        return 2;
    }
    const std::uint32_t entry_ip = static_cast<std::uint32_t>(std::distance(source_body.begin(), marker));

    std::vector<std::uint8_t> staged_space(static_cast<std::size_t>(main_script_size) + metadata.largest_mission_script_size, 0);
    std::copy_n(source_body.begin(), main_script_size, staged_space.begin());
    vcs::vm::ScriptVm vm(vcs::vm::ScriptSpace(std::move(staged_space), main_script_size));
    install_bootstrap_recording_adapters(vm);
    std::size_t synthetic_pickup_calls = 0;
    if (synthetic_pickup_dry_run) {
        // Exploration only: this is deliberately not evidence about VCS model
        // tables, terrain or pickup handles. It merely lets a caller discover
        // the next static-research wall after the fully proved 013F ABI.
        vm.set_pickup_model_object_resolver([](std::int32_t selector) -> std::optional<std::int32_t> {
            return selector;
        });
        vm.set_pickup_ground_height_probe([](float, float) -> std::optional<float> {
            return 0.0F;
        });
        vm.set_pickup_create_adapter([&](std::int32_t, std::uint8_t, std::int32_t, float, float, float) {
            return static_cast<std::int32_t>(0x4000 + synthetic_pickup_calls++);
        });
        vm.set_unique_jumps_total_adapter([](std::int32_t) {
            // Census-only sink; it does not represent native stat persistence.
        });
        vm.set_script_termination_adapter([](vcs::vm::ScriptVm&, vcs::vm::ScriptThread&, bool) {
            // Recording-only dry-run scheduler handoff; no game state exists.
        });
    }

    std::ofstream trace;
    if (trace_path) {
        trace.open(*trace_path, std::ios::trunc);
        if (!trace) {
            std::cerr << "Cannot write trace: " << *trace_path << '\n';
            return 2;
        }
    }

    vcs::vm::ScriptThread parent;
    parent.ip = entry_ip;
    parent.locals.resize(kPortableLocalSlots);
    const RunOutcome parent_before = run_until_stop(vm, parent, parent_budget, "main_before_0289", trace ? &trace : nullptr);
    print_result("parent_before_0289", parent_before, parent);
    if (parent_before.result.fault != vcs::vm::VmFault::Opcode0289RequiresMissionStreamingSubsystem) {
        std::cerr << "Expected explicit 0289 boundary before mission transfer\n";
        return 1;
    }

    // The core intentionally stops after consuming the one selector. Re-read
    // only that proven parameter from the saved opcode boundary; this mirrors
    // the handler decoder without exporting SCM payload.
    vcs::vm::ScriptThread selector_probe = parent;
    selector_probe.ip = parent_before.last_ip_before + 2;
    vcs::vm::VmFault selector_fault = vcs::vm::VmFault::None;
    const std::optional<std::int32_t> raw_selector = vm.read_parameter(selector_probe, selector_fault);
    if (!raw_selector || selector_fault != vcs::vm::VmFault::None || selector_probe.ip != parent.ip) {
        std::cerr << "Could not re-decode the proved 0289 selector boundary\n";
        return 1;
    }

    std::int64_t selector = *raw_selector;
    if (mission_selector_override) {
        // This is a read-only host-census selector, deliberately distinct
        // from the actual parent instruction just decoded above. It never
        // claims native reachability or rewrites source bytecode.
        selector = *mission_selector_override;
        std::cout << "mission_selector_override=" << selector
                  << " (read-only host census; parent decoded " << *raw_selector << ")\n";
    }
    if (host_mission_mode > 0) {
        if (selector < 0xFFFE) {
            std::cout << "mission_selector=" << selector << " host_mission_mode=" << host_mission_mode
                      << " native_guard=skip_stream\n";
            return 0;
        }
        selector = 0xFFFF - selector;
    }
    if (selector < 0 || static_cast<std::size_t>(selector) + 1 >= metadata.offsets.size()) {
        std::cerr << "Mission selector is outside the proven selector+1 table range\n";
        return 1;
    }
    const std::size_t mission_index = static_cast<std::size_t>(selector);
    const std::uint32_t mission_start = metadata.offsets[mission_index];
    const std::uint32_t mission_next = metadata.offsets[mission_index + 1];
    const std::uint32_t mission_size = mission_next > mission_start
        ? mission_next - mission_start
        : metadata.largest_mission_script_size;
    if (mission_size == 0 || mission_start > source_body.size() || mission_size > source_body.size() - mission_start ||
        mission_size > metadata.largest_mission_script_size) {
        std::cerr << "Mission source slice violates the recovered buffer contract\n";
        return 1;
    }

    const std::vector<std::uint8_t> mission_slice(
        source_body.begin() + static_cast<std::ptrdiff_t>(mission_start),
        source_body.begin() + static_cast<std::ptrdiff_t>(mission_start + mission_size));
    if (!vm.script_space().write_bytes_at(main_script_size, mission_slice)) {
        std::cerr << "Could not stage selected mission slice in the owned active buffer\n";
        return 1;
    }
    const std::size_t globals_start = static_cast<std::size_t>(metadata.num_true_globals) * 4 + 8;
    const std::size_t globals_size = static_cast<std::size_t>(metadata.most_globals) * 4;
    if (!vm.script_space().write_bytes_at(static_cast<std::uint32_t>(globals_start),
                                          std::vector<std::uint8_t>(globals_size, 0))) {
        std::cerr << "Could not clear the proved mission global region\n";
        return 1;
    }

    // The direct native factory is now proven as CTheScripts::StartNewScript:
    // idle-list removal, next-process-id assignment, Init, IP write,
    // active-list insertion and +0x208 = 1. This minimal harness does not
    // pretend to own those native lists/pool; it models only the selected
    // child's VM-visible fields before the proven 0289 marker writes.
    vcs::vm::ScriptThread mission;
    mission.ip = main_script_size;
    mission.saved_ip = main_script_size;
    mission.locals.resize(kPortableLocalSlots);
    mission.mission_launch_requested = true;
    mission.is_mission_script = true;

    std::cout << "mission_selector=" << selector << " host_mission_mode=" << host_mission_mode
              << " table_count=" << metadata.offsets.size()
              << " source_body_offset=0x" << std::hex << std::uppercase << mission_start
              << " source_file_offset=0x" << mission_start + 8
              << " copy_size=0x" << mission_size
              << " active_buffer_ip=0x" << main_script_size
              << std::dec << '\n';
    std::cout << "metadata variable_space_size=0x" << std::hex << std::uppercase << metadata.variable_space_size
              << " object_size=0x" << metadata.object_size
              << " num_true_globals=" << std::dec << metadata.num_true_globals
              << " most_globals=" << metadata.most_globals
              << " largest_mission_size=0x" << std::hex << std::uppercase << metadata.largest_mission_script_size
              << std::dec << " exclusive_mission_count=" << metadata.exclusive_mission_count
              << " globals_clear_start=0x" << std::hex << std::uppercase << globals_start
              << " globals_clear_size=0x" << globals_size << std::dec << '\n';
    if (synthetic_pickup_dry_run)
        std::cout << "synthetic_pickup_dry_run=true (exploration only; no host semantic claim)\n";

    // This is the native re-entrant Process(target) ordering. The result is a
    // new execution boundary, not a claim that all game-engine adapters exist.
    const RunOutcome mission_result = run_until_stop(vm, mission, mission_budget, "mission_reentrant", trace ? &trace : nullptr);
    print_result("mission_reentrant", mission_result, mission);
    if (synthetic_pickup_dry_run)
        std::cout << "synthetic_pickup_calls=" << synthetic_pickup_calls << '\n';

    // Native 0289 returns CONTINUE after target Process(), so demonstrate the
    // parent can resume at its already-advanced post-selector IP.
    const RunOutcome parent_after = run_until_stop(vm, parent, parent_resume_budget, "main_after_0289", trace ? &trace : nullptr);
    print_result("parent_after_0289", parent_after, parent);
    if (parent_after.result.fault == vcs::vm::VmFault::Opcode0289RequiresMissionStreamingSubsystem) {
        vcs::vm::ScriptThread next_selector_probe = parent;
        next_selector_probe.ip = parent_after.last_ip_before + 2;
        vcs::vm::VmFault next_selector_fault = vcs::vm::VmFault::None;
        const std::optional<std::int32_t> next_selector =
            vm.read_parameter(next_selector_probe, next_selector_fault);
        if (next_selector && next_selector_fault == vcs::vm::VmFault::None &&
            next_selector_probe.ip == parent.ip) {
            std::cout << "next_parent_0289_selector=" << *next_selector
                      << " (not streamed: harness proves one transfer per run)\n";
        }
    }

    return 0;
}
