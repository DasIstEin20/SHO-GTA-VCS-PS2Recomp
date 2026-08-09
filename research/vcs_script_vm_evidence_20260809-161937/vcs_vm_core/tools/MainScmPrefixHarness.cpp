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

const char* fault_name(vcs::vm::VmFault fault) {
    using vcs::vm::VmFault;
    switch (fault) {
    case VmFault::None: return "None";
    case VmFault::InstructionBudgetExhausted: return "InstructionBudgetExhausted";
    case VmFault::UnsupportedOpcode: return "UnsupportedOpcode";
    case VmFault::TruncatedBytecode: return "TruncatedBytecode";
    case VmFault::Opcode0289RequiresMissionStreamingSubsystem: return "Opcode0289RequiresMissionStreamingSubsystem";
    case VmFault::Opcode00C9RequiresFadeStartAdapter: return "Opcode00C9RequiresFadeStartAdapter";
    default: return "OtherVmFault";
    }
}

const char* status_name(vcs::vm::NativeHandlerStatus status) {
    return status == vcs::vm::NativeHandlerStatus::Continue ? "CONTINUE" : "YIELD";
}

bool parse_command_count(const char* text, std::size_t& out) {
    try {
        const std::string value(text);
        std::size_t parsed = 0;
        const unsigned long long number = std::stoull(value, &parsed, 10);
        if (parsed != value.size() || number == 0 || number > 10000)
            return false;
        out = static_cast<std::size_t>(number);
        return true;
    } catch (...) {
        return false;
    }
}

std::uint32_t read_u32_le(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
}

std::string name_as_text(const std::array<char, 8>& name) {
    const auto end = std::find(name.begin(), name.end(), '\0');
    return std::string(name.begin(), end);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: vcs_vm_main_scm_prefix_harness <MAIN.SCM> [--commands N] [--trace-jsonl PATH]\n";
        return 2;
    }

    std::size_t command_count = 5;
    std::optional<std::string> trace_path;
    for (int index = 2; index < argc; ) {
        const std::string option(argv[index++]);
        if (option == "--commands" && index < argc) {
            if (!parse_command_count(argv[index++], command_count)) {
                std::cerr << "--commands must be a decimal integer in 1..10000\n";
                return 2;
            }
        } else if (option == "--trace-jsonl" && index < argc) {
            trace_path = argv[index++];
        } else {
            std::cerr << "Usage: vcs_vm_main_scm_prefix_harness <MAIN.SCM> [--commands N] [--trace-jsonl PATH]\n";
            return 2;
        }
    }

    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Cannot open MAIN.SCM read-only: " << argv[1] << '\n';
        return 2;
    }
    const std::vector<std::uint8_t> bytes{
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (bytes.size() < 8) {
        std::cerr << "MAIN.SCM is shorter than its VCS header\n";
        return 2;
    }
    const std::uint32_t main_segment_size = read_u32_le(bytes, 0);
    if (main_segment_size > bytes.size() - 8) {
        std::cerr << "Invalid VCS main-segment boundary\n";
        return 2;
    }

    // The first eight file bytes are the VCS SCM header, not ScriptSpace.
    // Native thread IPs and in-bytecode GOSUB targets are relative to the
    // first byte after that header.  Keeping the header in ScriptSpace shifts
    // every direct target by eight and can turn a valid command into adjacent
    // text data (the first MAIN path exposes this immediately at 01BA).
    const std::vector<std::uint8_t> script_bytes(bytes.begin() + 8, bytes.end());
    // Header dword 0 is already a byte count relative to ScriptSpace, i.e.
    // the number of post-header MAIN bytes.  Subtracting the file header a
    // second time would misplace the mission-buffer boundary by eight bytes.
    const std::uint32_t script_main_segment_end = main_segment_size;
    const auto marker = std::search(
        script_bytes.begin(), script_bytes.begin() + static_cast<std::ptrdiff_t>(script_main_segment_end),
        kMainEntryMarker.begin(), kMainEntryMarker.end());
    if (marker == script_bytes.begin() + static_cast<std::ptrdiff_t>(script_main_segment_end)) {
        std::cerr << "Could not find unique SCRIPT_NAME MAIN entry marker\n";
        return 2;
    }
    if (std::search(marker + 1, script_bytes.begin() + static_cast<std::ptrdiff_t>(script_main_segment_end),
                    kMainEntryMarker.begin(), kMainEntryMarker.end()) !=
        script_bytes.begin() + static_cast<std::ptrdiff_t>(script_main_segment_end)) {
        std::cerr << "SCRIPT_NAME MAIN marker is not unique\n";
        return 2;
    }

    const auto entry = static_cast<std::uint32_t>(std::distance(script_bytes.begin(), marker));
    vcs::vm::ScriptVm vm(vcs::vm::ScriptSpace(script_bytes, script_main_segment_end));
    vm.set_clock([] { return 0U; });
    std::size_t fade_calls = 0;
    float fade_seconds = 0.0F;
    std::int16_t fade_mode = 0;
    vm.set_fade_start_adapter([&](float seconds, std::int16_t mode) {
        ++fade_calls;
        fade_seconds = seconds;
        fade_mode = mode;
    });
    std::size_t pause_menu_calls = 0;
    bool pause_menu_parameter_is_nonzero = false;
    vm.set_pause_menu_toggle_adapter([&](bool parameter_is_nonzero) {
        ++pause_menu_calls;
        pause_menu_parameter_is_nonzero = parameter_is_nonzero;
    });
    std::size_t display_text_calls = 0;
    vm.set_display_text_adapter([&](std::int32_t, std::int32_t, std::int32_t) {
        ++display_text_calls;
    });

    vcs::vm::ScriptThread thread;
    thread.ip = entry;
    vcs::vm::ProcessResult result{};
    if (!trace_path) {
        result = vm.process(thread, command_count);
    } else {
        std::ofstream trace(*trace_path, std::ios::trunc);
        if (!trace) {
            std::cerr << "Cannot write trace: " << *trace_path << '\n';
            return 2;
        }
        for (std::size_t count = 0; count < command_count; ++count) {
            const std::uint32_t ip_before = thread.ip;
            std::uint32_t probe_ip = ip_before;
            std::int16_t raw_signed = 0;
            const bool raw_present = vm.script_space().read_i16(probe_ip, raw_signed);
            result = vm.process_one_command(thread);
            result.executed = count + 1;
            trace << "{\"index\":" << count
                  << ",\"ip_before\":\"0x" << std::hex << std::uppercase << ip_before << std::dec
                  << "\",\"raw_opcode\":";
            if (raw_present) {
                const auto raw = static_cast<std::uint16_t>(raw_signed);
                trace << "\"0x" << std::hex << std::uppercase << raw << std::dec << "\"";
            } else {
                trace << "null";
            }
            trace << ",\"opcode\":\"0x" << std::hex << std::uppercase << result.last_opcode << std::dec
                  << "\",\"ip_after\":\"0x" << std::hex << std::uppercase << thread.ip << std::dec
                  << "\",\"status\":\"" << status_name(result.status)
                  << "\",\"fault\":\"" << fault_name(result.fault) << "\"}\n";
            if (result.fault != vcs::vm::VmFault::None ||
                result.status != vcs::vm::NativeHandlerStatus::Continue)
                break;
        }
        if (result.executed == command_count && result.fault == vcs::vm::VmFault::None &&
            result.status == vcs::vm::NativeHandlerStatus::Continue) {
            result.status = vcs::vm::NativeHandlerStatus::Yield;
            result.fault = vcs::vm::VmFault::InstructionBudgetExhausted;
        }
    }

    std::cout << "entry_ip=0x" << std::hex << std::uppercase << entry
              << " final_ip=0x" << thread.ip
              << " last_opcode=0x" << std::setw(4) << std::setfill('0') << result.last_opcode
              << std::dec << std::setfill(' ') << '\n';
    std::cout << "executed=" << result.executed
              << " fault=" << fault_name(result.fault)
              << " script_name=" << name_as_text(thread.script_name)
              << " call_depth=" << thread.call_stack.size()
              << " recorded_do_fade_calls=" << fade_calls;
    if (fade_calls != 0)
        std::cout << " fade_seconds=" << fade_seconds << " fade_mode=" << fade_mode;
    std::cout << " recorded_pause_menu_calls=" << pause_menu_calls;
    if (pause_menu_calls != 0)
        std::cout << " pause_menu_parameter_is_nonzero=" << std::boolalpha << pause_menu_parameter_is_nonzero;
    std::cout << " recorded_display_text_calls=" << display_text_calls;
    std::cout << '\n';

    // Reaching the requested instruction cap is the successful bounded-prefix
    // outcome. Any other result is left visible as a non-zero exit, including
    // an unimplemented next opcode when the user asks for a longer slice.
    return result.fault == vcs::vm::VmFault::InstructionBudgetExhausted &&
            result.executed == command_count
        ? 0
        : 1;
}
