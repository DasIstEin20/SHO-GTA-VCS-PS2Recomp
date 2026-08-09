#include "vcs_host/VcsDiscFs.h"
#include "vcs_host/VcsMissionHost.h"
#include "vcs_vm/ScriptVmCore.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kPortableLocalSlots = 106;

const char* fault_name(vcs::vm::VmFault fault) {
    using vcs::vm::VmFault;
    switch (fault) {
    case VmFault::None: return "None";
    case VmFault::InstructionBudgetExhausted: return "InstructionBudgetExhausted";
    case VmFault::UnsupportedOpcode: return "UnsupportedOpcode";
    case VmFault::Opcode013FRequiresPickupModelObjectResolver: return "Opcode013FRequiresPickupModelObjectResolver";
    case VmFault::Opcode013FRequiresPickupGroundHeightProbe: return "Opcode013FRequiresPickupGroundHeightProbe";
    case VmFault::Opcode013FRequiresPickupCreateAdapter: return "Opcode013FRequiresPickupCreateAdapter";
    case VmFault::Opcode01E7RequiresUniqueJumpsTotalAdapter: return "Opcode01E7RequiresUniqueJumpsTotalAdapter";
    case VmFault::Opcode0289RequiresMissionStreamingSubsystem: return "Opcode0289RequiresMissionStreamingSubsystem";
    case VmFault::InvalidScriptTarget: return "InvalidScriptTarget";
    default: return "OtherVmFault";
    }
}

bool parse_positive_size(const char* text, std::size_t& value) {
    try {
        const std::string input(text);
        std::size_t consumed = 0;
        const unsigned long long parsed = std::stoull(input, &consumed, 10);
        if (consumed != input.size() || parsed == 0 || parsed > 1000000)
            return false;
        value = static_cast<std::size_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_i32(const char* text, std::int32_t& value) {
    try {
        const std::string input(text);
        std::size_t consumed = 0;
        const long long parsed = std::stoll(input, &consumed, 10);
        if (consumed != input.size() || parsed < std::numeric_limits<std::int32_t>::min() ||
            parsed > std::numeric_limits<std::int32_t>::max())
            return false;
        value = static_cast<std::int32_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

void install_bootstrap_recording_adapters(vcs::vm::ScriptVm& vm) {
    // Only already-proved bootstrap adapters. They record no assets and do
    // not imitate a renderer, streaming system or native game state.
    vm.set_clock([] { return 0U; });
    vm.set_fade_start_adapter([](float, std::int16_t) {});
    vm.set_pause_menu_toggle_adapter([](bool) {});
    vm.set_display_text_adapter([](std::int32_t, std::int32_t, std::int32_t) {});
}

void print_usage() {
    std::cerr << "Usage: gta-vcs-bootstrap <VCS_PS2.iso|extracted-disc-dir>"
              << " [--main ISO/PATH/MAIN.SCM] [--budget N] [--child-budget N] [--mission-mode N]"
              << " [--require-known-ps2-layout]\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 2;
    }
    std::string source_path(argv[1]);
    std::optional<std::string> requested_main;
    std::size_t parent_budget = 1000;
    std::size_t child_budget = 1000;
    std::int32_t mission_mode = 0;
    bool require_known_ps2_layout = false;
    for (int index = 2; index < argc; ) {
        const std::string option(argv[index++]);
        if (option == "--main" && index < argc) {
            requested_main = argv[index++];
        } else if (option == "--budget" && index < argc) {
            if (!parse_positive_size(argv[index++], parent_budget)) {
                print_usage();
                return 2;
            }
        } else if (option == "--child-budget" && index < argc) {
            if (!parse_positive_size(argv[index++], child_budget)) {
                print_usage();
                return 2;
            }
        } else if (option == "--mission-mode" && index < argc) {
            if (!parse_i32(argv[index++], mission_mode)) {
                print_usage();
                return 2;
            }
        } else if (option == "--require-known-ps2-layout") {
            require_known_ps2_layout = true;
        } else {
            print_usage();
            return 2;
        }
    }

    std::string error;
    const std::unique_ptr<vcs::host::IVcsFileSystem> fs = vcs::host::open_vcs_disc_source(source_path, error);
    if (!fs) {
        std::cerr << "FAIL source: " << error << '\n';
        return 2;
    }

    const std::vector<std::string> candidates = requested_main
        ? std::vector<std::string>{*requested_main}
        : std::vector<std::string>{"MAIN.SCM", "RUNDATA/MAIN.SCM", "PSP_GAME/USRDIR/RUNDATA/MAIN.SCM"};
    std::optional<vcs::host::VcsScriptContainer> container;
    std::string selected_main;
    for (const std::string& candidate : candidates) {
        error.clear();
        container = vcs::host::load_vcs_script_container(*fs, candidate, error);
        if (container) {
            selected_main = candidate;
            break;
        }
    }
    if (!container) {
        std::cerr << "FAIL SCM container: " << error << '\n';
        return 2;
    }
    const std::optional<std::uint32_t> entry_ip = container->find_main_entry();
    if (!entry_ip) {
        std::cerr << "FAIL SCM container: unique SCRIPT_NAME MAIN entry was not found\n";
        return 2;
    }
    if (require_known_ps2_layout && !container->matches_known_vcs_ps2_sles_546_22_layout()) {
        std::cerr << "FAIL SCM identity: expected the evidenced VCS PS2 SLES_546.22 container layout, got "
                  << container->layout_profile() << '\n';
        return 2;
    }
    const std::uint64_t staged_size = static_cast<std::uint64_t>(container->main_script_size()) +
        container->largest_mission_script_size();
    if (staged_size > std::numeric_limits<std::uint32_t>::max()) {
        std::cerr << "FAIL SCM container: active ScriptSpace size overflows portable VM\n";
        return 2;
    }
    std::vector<std::uint8_t> staged(static_cast<std::size_t>(staged_size), 0);
    std::copy_n(container->source_body().begin(), container->main_script_size(), staged.begin());
    vcs::vm::ScriptVm vm(vcs::vm::ScriptSpace(std::move(staged), container->main_script_size()));
    install_bootstrap_recording_adapters(vm);

    vcs::host::VcsMissionHost mission_host(std::move(*container), 64, child_budget);
    mission_host.set_mission_mode(mission_mode);
    mission_host.bind(vm);

    vcs::vm::ScriptThread main_thread;
    main_thread.ip = *entry_ip;
    main_thread.saved_ip = *entry_ip;
    main_thread.locals.resize(kPortableLocalSlots);
    const vcs::vm::ProcessResult result = vm.process(main_thread, parent_budget);
    const vcs::host::MissionLaunchSnapshot state = mission_host.snapshot();

    std::cout << "source=" << fs->description() << " scm=" << selected_main
              << " layout=" << mission_host.container().layout_profile()
              << " main_size=0x" << std::hex << std::uppercase << mission_host.container().main_script_size()
              << " entry_ip=0x" << *entry_ip
              << " mission_count=" << std::dec << mission_host.container().mission_metadata().offsets.size() << '\n';
    std::cout << "parent executed=" << result.executed
              << " last_opcode=0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
              << result.last_opcode << std::dec << std::setfill(' ')
              << " ip=0x" << std::hex << std::uppercase << main_thread.ip << std::dec
              << " fault=" << fault_name(result.fault) << '\n';
    std::cout << "mission_host active=" << std::boolalpha << state.mission_active
              << " scope_depth=" << state.stream_scope_depth
              << " active_threads=" << state.active_thread_count
              << " idle_threads=" << state.idle_thread_count;
    if (state.last_selector)
        std::cout << " selector=" << *state.last_selector;
    std::cout << '\n';
    if (state.last_child_result) {
        std::cout << "child executed=" << state.last_child_result->executed
                  << " last_opcode=0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
                  << state.last_child_result->last_opcode << std::dec << std::setfill(' ')
                  << " fault=" << fault_name(state.last_child_result->fault) << '\n';
    }

    if (result.fault == vcs::vm::VmFault::None) {
        std::cout << "PASS bootstrap stayed within implemented host/VM surface\n";
        return 0;
    }
    std::cout << "FAIL LOUDLY first unimplemented boundary is shown above; no game state was fabricated\n";
    return 3;
}
