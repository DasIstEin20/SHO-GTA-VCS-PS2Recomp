#include "vcs_host/VcsMissionHost.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

namespace {

void write_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8U);
}

void write_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3] = static_cast<std::uint8_t>(value >> 24U);
}

vcs::host::VcsScriptContainer make_minimal_container() {
    constexpr std::uint32_t kMainSize = 0x60;
    constexpr std::uint32_t kMissionSize = 0x10;
    std::vector<std::uint8_t> body(kMainSize + kMissionSize, 0);

    // ReadMultiScriptFileOffsetsFromScript shape: variable_size=0,
    // object_size=0, metadata at +8 and two offset-table entries at +20.
    write_u32(body, 3, 0);
    write_u16(body, 8, 0); // NumTrueGlobals
    write_u16(body, 10, 1); // MostGlobals, so the host must clear four bytes
    write_u32(body, 12, kMissionSize);
    write_u16(body, 16, 2); // mission count
    write_u16(body, 18, 0);
    write_u32(body, 20, kMainSize);
    write_u32(body, 24, kMainSize + kMissionSize);

    const std::vector<std::uint8_t> entry{
        0x38, 0x02, 0x0A, 'M', 'A', 'I', 'N', 0x00, // SCRIPT_NAME MAIN
        0x89, 0x02, 0x01, // 0289 with raw selector zero
    };
    for (std::size_t index = 0; index < entry.size(); ++index)
        body[0x40 + index] = entry[index];
    // Child waits zero milliseconds, therefore its re-entrant Process yields
    // successfully and 0289 itself remains a native CONTINUE.
    body[kMainSize + 0] = 0x01;
    body[kMainSize + 1] = 0x00;
    body[kMainSize + 2] = 0x01;

    std::vector<std::uint8_t> file(8 + body.size(), 0);
    write_u32(file, 0, kMainSize);
    write_u32(file, 4, kMissionSize);
    for (std::size_t index = 0; index < body.size(); ++index)
        file[8 + index] = body[index];
    std::string error;
    const auto parsed = vcs::host::VcsScriptContainer::parse(std::move(file), error);
    assert(parsed && error.empty());
    return *parsed;
}

void test_0289_host_preserves_stream_factory_and_reentrant_process_contract() {
    vcs::host::VcsScriptContainer container = make_minimal_container();
    const auto entry = container.find_main_entry();
    assert(entry && *entry == 0x40);

    std::vector<std::uint8_t> staged(container.main_script_size() + container.largest_mission_script_size(), 0);
    for (std::size_t index = 0; index < container.main_script_size(); ++index)
        staged[index] = container.source_body()[index];
    vcs::vm::ScriptVm vm(vcs::vm::ScriptSpace(std::move(staged), container.main_script_size()));
    vm.set_clock([] { return 0U; });
    vcs::host::VcsMissionHost host(std::move(container), 2, 10);
    host.bind(vm);

    vcs::vm::ScriptThread parent;
    parent.ip = *entry;
    parent.locals.resize(106);
    const vcs::vm::ProcessResult result = vm.process(parent, 2);

    // The bounded outer loop sees SCRIPT_NAME then 0289. The child did not
    // fault; the budget result confirms that 0289 returned native CONTINUE.
    assert(result.fault == vcs::vm::VmFault::InstructionBudgetExhausted);
    assert(result.last_opcode == 0x0289);
    const vcs::host::MissionLaunchSnapshot state = host.snapshot();
    assert(state.mission_active);
    assert(state.stream_scope_depth == 0);
    assert(state.next_process_id == 1);
    assert(state.active_thread_count == 1 && state.idle_thread_count == 1);
    assert(state.last_selector && *state.last_selector == 0);
    assert(state.last_child_scheduler_id && *state.last_child_scheduler_id == 0);
    assert(state.last_child_result);
    assert(state.last_child_result->fault == vcs::vm::VmFault::None);
    assert(state.last_child_result->last_opcode == 0x0001);
    assert(state.last_child_result->status == vcs::vm::NativeHandlerStatus::Yield);

    std::int32_t cleared_globals = -1;
    assert(vm.script_space().read_i32_at(8, cleared_globals));
    assert(cleared_globals == 0);
}

} // namespace

int main() {
    test_0289_host_preserves_stream_factory_and_reentrant_process_contract();
    std::cout << "vcs_mission_host tests passed\n";
}
