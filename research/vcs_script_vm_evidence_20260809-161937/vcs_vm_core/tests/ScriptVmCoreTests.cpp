#include "vcs_vm/ScriptVmCore.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

namespace {

using vcs::vm::NativeHandlerStatus;
using vcs::vm::ProcessResult;
using vcs::vm::ScriptSpace;
using vcs::vm::ScriptThread;
using vcs::vm::ScriptVm;
using vcs::vm::VmFault;

void append_u16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
}

void append_i16(std::vector<std::uint8_t>& bytes, std::int16_t value) {
    append_u16(bytes, static_cast<std::uint16_t>(value));
}

void append_i32(std::vector<std::uint8_t>& bytes, std::int32_t value) {
    const auto raw = static_cast<std::uint32_t>(value);
    for (unsigned int shift = 0; shift < 32; shift += 8)
        bytes.push_back(static_cast<std::uint8_t>(raw >> shift));
}

void test_dispatcher_strips_not_bit_and_nop_continues() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x8000); // NOT | NOP
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;

    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None);
    assert(result.last_opcode == 0x0000);
    assert(result.status == NativeHandlerStatus::Continue);
    assert(thread.not_flag);
    assert(thread.ip == 2);
}

void test_wait_yields_and_uses_the_immediate_delay() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0001);
    bytes.push_back(7); // ARGUMENT_INT8
    bytes.push_back(50);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    vm.set_clock([] { return 100U; });
    ScriptThread thread;
    thread.wait_flag = true;

    const ProcessResult result = vm.process(thread);
    assert(result.fault == VmFault::None);
    assert(result.status == NativeHandlerStatus::Yield);
    assert(result.executed == 1);
    assert(thread.ip == 4);
    assert(thread.wake_time == 150);
    assert(!thread.wait_flag);
}

void test_goto_sets_ip_and_continues() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0002);
    bytes.push_back(6); // ARGUMENT_INT32
    append_i32(bytes, 7);
    bytes.push_back(0); // valid destination byte at IP 7
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;

    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None);
    assert(result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 7);
}

void test_start_new_script_clamps_index_and_copies_96_locals_through_host() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0024);
    bytes.push_back(6); // requested script index
    append_i32(bytes, -7);
    for (std::int32_t value = 0; value < 96; ++value) {
        bytes.push_back(6);
        append_i32(bytes, value * 3 - 10);
    }

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread child;
    child.locals.resize(3, 1234);
    bool adapter_called = false;
    vm.set_start_new_script_adapter([&](ScriptVm&, std::int32_t script_index) -> ScriptThread* {
        adapter_called = true;
        assert(script_index == 0); // Native handler clamps negative index to zero.
        return &child;
    });
    ScriptThread parent;
    const ProcessResult result = vm.process_one_command(parent);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(adapter_called && parent.ip == 487);
    assert(child.locals.size() >= 96);
    assert(child.locals[0] == -10 && child.locals[1] == -7 && child.locals[95] == 275);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0024);
    missing_adapter_bytes.push_back(1); // INT_ZERO makes the first parameter valid.
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_parent;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_parent);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0024RequiresStartNewScriptAdapter);
}

void test_numeric_conversions_and_absolute_value_handlers() {
    struct ConversionCase {
        std::uint16_t opcode;
        std::int32_t source;
        std::int32_t expected;
    };
    for (const ConversionCase& item : std::vector<ConversionCase>{
             {0x0038, static_cast<std::int32_t>(0xC0600000U), -3}, // trunc.w.s(-3.5) = -3, not floor = -4
             {0x0039, -7, static_cast<std::int32_t>(0xC0E00000U)}, // int -7 -> binary32 -7.0
         }) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, item.opcode);
        bytes.push_back(0x0D); // destination local 0
        bytes.push_back(0x0E); // source local 1
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals = {0, item.source};
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(thread.locals[0] == item.expected);
    }

    for (const auto& item : std::vector<std::pair<std::uint16_t, std::int32_t>>{
             {0x003A, -13},
             {0x003A, std::numeric_limits<std::int32_t>::min()}, // native subu preserves INT_MIN
             {0x003B, static_cast<std::int32_t>(0xC0600000U)}, // -3.5 -> +3.5
             {0x003B, static_cast<std::int32_t>(0x80000000U)}, // -0 remains -0: c.lt.s is false
         }) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, item.first);
        bytes.push_back(0x0D);
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals = {item.second};
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        if (item.first == 0x003A)
            assert(thread.locals[0] == (item.second == std::numeric_limits<std::int32_t>::min() ? item.second : 13));
        else if (static_cast<std::uint32_t>(item.second) == 0xC0600000U)
            assert(static_cast<std::uint32_t>(thread.locals[0]) == 0x40600000U);
        else
            assert(static_cast<std::uint32_t>(thread.locals[0]) == 0x80000000U);
    }

    std::vector<std::uint8_t> nan_bytes;
    append_u16(nan_bytes, 0x0038);
    nan_bytes.push_back(0x0D);
    nan_bytes.push_back(0x0E);
    ScriptVm nan_vm(ScriptSpace(std::move(nan_bytes)));
    ScriptThread nan_thread;
    nan_thread.locals = {99, static_cast<std::int32_t>(0x7FC00000U)};
    const ProcessResult nan_result = nan_vm.process_one_command(nan_thread);
    assert(nan_result.status == NativeHandlerStatus::Yield);
    assert(nan_result.fault == VmFault::FloatToIntegerConversionOutOfRange);
    assert(nan_thread.locals[0] == 99);
}

void test_timed_float_arithmetic_uses_host_time_step() {
    for (const std::uint16_t opcode : std::vector<std::uint16_t>{0x0031, 0x0032, 0x0033, 0x0034}) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, opcode);
        bytes.push_back(0x0D); // destination local 0
        bytes.push_back(9);    // one raw binary32 parameter (8.0)
        append_i32(bytes, static_cast<std::int32_t>(0x41000000U));
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        vm.set_time_step_provider([] { return 0.25F; });
        ScriptThread thread;
        thread.locals = {static_cast<std::int32_t>(0x3F800000U)}; // 1.0
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        const std::uint32_t expected = (opcode == 0x0031 || opcode == 0x0032)
            ? 0x40400000U  // 1 + 8 * .25 = 3
            : 0xBF800000U; // 1 - 8 * .25 = -1
        assert(static_cast<std::uint32_t>(thread.locals[0]) == expected);
    }

    std::vector<std::uint8_t> missing_provider_bytes;
    append_u16(missing_provider_bytes, 0x0031);
    missing_provider_bytes.push_back(0x0D);
    missing_provider_bytes.push_back(1); // INT_ZERO; decoding is valid before the host boundary.
    ScriptVm missing_provider_vm(ScriptSpace(std::move(missing_provider_bytes)));
    ScriptThread missing_provider_thread;
    missing_provider_thread.locals = {static_cast<std::int32_t>(0x3F800000U)};
    const ProcessResult missing_provider_result = missing_provider_vm.process_one_command(missing_provider_thread);
    assert(missing_provider_result.status == NativeHandlerStatus::Yield);
    assert(missing_provider_result.fault == VmFault::TimedArithmeticRequiresTimeStep);
    assert(static_cast<std::uint32_t>(missing_provider_thread.locals[0]) == 0x3F800000U);
}

void test_assign_aliases_store_raw_parameter_dwords() {
    for (const auto opcode : std::vector<std::uint16_t>{0x0004, 0x0005, 0x0006, 0x0035, 0x0036, 0x0037, 0x02E2}) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, opcode);
        bytes.push_back(0x0D); // local at LP + 0: output destination
        bytes.push_back(9);    // FLOAT raw bits: a dword assignment is type-agnostic
        append_i32(bytes, static_cast<std::int32_t>(0x7FC01234U));
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals.resize(1);
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(static_cast<std::uint32_t>(thread.locals[0]) == 0x7FC01234U);
    }
}

void test_set_var_text_label_stores_the_collected_scriptspace_string_offset() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0006);
    bytes.push_back(0x0D); // local destination
    bytes.push_back(10);   // ScriptSpace NUL-terminated literal string
    bytes.insert(bytes.end(), {'V', 'C', 'S', 0});
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    thread.locals.resize(1);
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.locals[0] == 4 && thread.ip == 8);
}

void test_add_value_to_int_var_uses_modulo_32_bit_math() {
    std::vector<std::uint8_t> bytes;
    for (const auto opcode : std::vector<std::uint16_t>{0x0007, 0x0029}) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, opcode);
        bytes.push_back(0x0D); // local at LP + 0
        bytes.push_back(6);    // INT32
        append_i32(bytes, 1);
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals.resize(1);
        thread.locals[0] = std::numeric_limits<std::int32_t>::max();
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(thread.locals[0] == std::numeric_limits<std::int32_t>::min());
    }
}

void test_remaining_scalar_arithmetic_handlers() {
    struct IntCase {
        std::uint16_t opcode;
        std::int32_t current;
        std::int32_t value;
        std::int32_t expected;
    };
    for (const IntCase& item : std::vector<IntCase>{
             {0x0009, 2, 5, -3},
             {0x002B, 2, 5, -3},
             {0x000B, -3, 7, -21},
             {0x002D, -3, 7, -21},
             {0x000D, -7, 2, -3}, // MIPS/C++ signed division truncates toward zero
             {0x002F, -7, 2, -3},
         }) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, item.opcode);
        bytes.push_back(0x0D);
        bytes.push_back(6);
        append_i32(bytes, item.value);
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals.resize(1);
        thread.locals[0] = item.current;
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(thread.locals[0] == item.expected);
    }

    struct FloatCase {
        std::uint16_t opcode;
        std::uint32_t current_bits;
        std::uint32_t value_bits;
        std::uint32_t expected_bits;
    };
    for (const FloatCase& item : std::vector<FloatCase>{
             {0x0008, 0x3FC00000U, 0x40100000U, 0x40700000U}, // 1.5 + 2.25 = 3.75
             {0x002A, 0x3FC00000U, 0x40100000U, 0x40700000U},
             {0x000A, 0x40600000U, 0x3F800000U, 0x40200000U}, // 3.5 - 1.0 = 2.5
             {0x002C, 0x40600000U, 0x3F800000U, 0x40200000U},
             {0x000C, 0x3FC00000U, 0x40000000U, 0x40400000U}, // 1.5 * 2.0 = 3.0
             {0x002E, 0x3FC00000U, 0x40000000U, 0x40400000U},
             {0x000E, 0x40E00000U, 0x40000000U, 0x40600000U}, // 7.0 / 2.0 = 3.5
             {0x0030, 0x40E00000U, 0x40000000U, 0x40600000U},
         }) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, item.opcode);
        bytes.push_back(0x0D);
        bytes.push_back(6);
        append_i32(bytes, static_cast<std::int32_t>(item.value_bits));
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals.resize(1);
        thread.locals[0] = static_cast<std::int32_t>(item.current_bits);
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(static_cast<std::uint32_t>(thread.locals[0]) == item.expected_bits);
    }

    std::vector<std::uint8_t> divide_by_zero;
    append_u16(divide_by_zero, 0x000D);
    divide_by_zero.push_back(0x0D);
    divide_by_zero.push_back(6);
    append_i32(divide_by_zero, 0);
    ScriptVm divide_vm(ScriptSpace(std::move(divide_by_zero)));
    ScriptThread divide_thread;
    divide_thread.locals.resize(1);
    divide_thread.locals[0] = 10;
    const ProcessResult divide_result = divide_vm.process_one_command(divide_thread);
    assert(divide_result.status == NativeHandlerStatus::Yield);
    assert(divide_result.fault == VmFault::IntegerDivisionByZero);
    assert(divide_thread.locals[0] == 10);
}

void test_integer_condition_handlers_use_signed_predicates_and_condition_reducer() {
    struct Case {
        std::uint16_t opcode;
        std::int32_t left;
        std::int32_t right;
        bool expected;
    };
    const std::vector<Case> cases{
        {0x000F, 9, 3, true},       // int var > number
        {0x0010, 3, 9, false},      // number > int var, same raw p0 > p1 form
        {0x0011, -3, -4, true},     // int var > int var
        {0x0015, -4, -4, true},     // signed >=
        {0x0016, -4, -3, false},    // number >= int var
        {0x0017, 9, 9, true},       // int var >= int var
        {0x001B, -8, -8, true},     // equality
        {0x001C, 8, -8, false},     // equality between variables
        {0x02DB, 7, 8, false},      // equality alias
    };
    for (const Case& item : cases) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, item.opcode);
        bytes.push_back(6); // INT32 parameter[0]
        append_i32(bytes, item.left);
        bytes.push_back(6); // INT32 parameter[1]
        append_i32(bytes, item.right);
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(thread.condition_result == item.expected);
        assert(thread.ip == 12);
    }

    // The shared reducer receives the raw dispatcher NOT bit and aggregates.
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x800F); // NOT | IS_INT_VAR_GREATER_THAN_NUMBER
    bytes.push_back(6);
    append_i32(bytes, 9);
    bytes.push_back(6);
    append_i32(bytes, 3);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    thread.condition_state = 2;
    thread.condition_result = true;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && thread.not_flag);
    assert(thread.condition_state == 1 && !thread.condition_result);
}

void test_float_condition_handlers_preserve_mips_unordered_and_raw_equality_rules() {
    struct Case {
        std::uint16_t opcode;
        std::uint32_t left_bits;
        std::uint32_t right_bits;
        bool expected;
    };
    const std::vector<Case> cases{
        {0x0012, 0x40400000U, 0x40000000U, true},  // 3.0 > 2.0
        {0x0013, 0x40000000U, 0x40400000U, false}, // 2.0 > 3.0
        {0x0014, 0xBF800000U, 0xC0000000U, true},  // -1.0 > -2.0
        {0x0018, 0x40000000U, 0x40000000U, true},  // 2.0 >= 2.0
        {0x0019, 0x3F800000U, 0x40000000U, false}, // 1.0 >= 2.0
        {0x001A, 0x40000000U, 0x40000000U, true},  // 2.0 >= 2.0
        // PSP c.le.s/c.lt.s followed by an inverted branch treats unordered
        // comparisons as true for both target families.
        {0x0012, 0x7FC00000U, 0x3F800000U, true},
        {0x0018, 0x7FC00000U, 0x3F800000U, true},
        // Equality uses the shared integer XOR handler, so it is bitwise.
        {0x001D, 0x00000000U, 0x80000000U, false}, // +0.0 != -0.0 in the VM
        {0x001E, 0x7FC00000U, 0x7FC00000U, true},  // same NaN payload matches
    };
    for (const Case& item : cases) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, item.opcode);
        bytes.push_back(6); // raw float parameter[0]
        append_i32(bytes, static_cast<std::int32_t>(item.left_bits));
        bytes.push_back(6); // raw float parameter[1]
        append_i32(bytes, static_cast<std::int32_t>(item.right_bits));
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        const ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
        assert(thread.condition_result == item.expected && thread.ip == 12);
    }
}

void test_is_char_still_alive_uses_host_predicate_and_exact_condition_vm() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x8045); // raw NOT must be applied by the common reducer
    bytes.push_back(6);
    append_i32(bytes, 0x1234);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    bool called = false;
    vm.set_opcode_0045_condition_probe([&](ScriptVm&, ScriptThread&, std::int32_t handle) {
        called = true;
        assert(handle == 0x1234);
        return true;
    });
    ScriptThread thread;
    thread.condition_state = 2;
    thread.condition_result = true;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(called && thread.not_flag && thread.condition_state == 1 && !thread.condition_result);

    std::vector<std::uint8_t> missing_probe_bytes;
    append_u16(missing_probe_bytes, 0x0045);
    missing_probe_bytes.push_back(1);
    ScriptVm missing_probe_vm(ScriptSpace(std::move(missing_probe_bytes)));
    ScriptThread missing_probe_thread;
    const ProcessResult missing_probe_result = missing_probe_vm.process_one_command(missing_probe_thread);
    assert(missing_probe_result.status == NativeHandlerStatus::Yield);
    assert(missing_probe_result.fault == VmFault::Opcode0045RequiresConditionProbe);
}

void test_is_button_pressed_truncates_two_inputs_and_uses_condition_vm() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x807F); // raw NOT must be applied after the host predicate
    bytes.push_back(6);
    append_i32(bytes, 0x12340002);
    bytes.push_back(6);
    append_i32(bytes, 0xABCD0040);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    bool called = false;
    vm.set_opcode_007f_button_pressed_probe(
        [&](ScriptVm&, ScriptThread&, std::uint16_t first, std::uint16_t second) {
            called = true;
            assert(first == 2 && second == 0x40);
            return true;
        });
    ScriptThread thread;
    thread.condition_state = 2;
    thread.condition_result = true;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(called && thread.not_flag && thread.condition_state == 1 && !thread.condition_result && thread.ip == 12);

    std::vector<std::uint8_t> missing_probe_bytes;
    append_u16(missing_probe_bytes, 0x007F);
    missing_probe_bytes.push_back(1);
    missing_probe_bytes.push_back(1);
    ScriptVm missing_probe_vm(ScriptSpace(std::move(missing_probe_bytes)));
    ScriptThread missing_probe_thread;
    const ProcessResult missing_probe_result = missing_probe_vm.process_one_command(missing_probe_thread);
    assert(missing_probe_result.status == NativeHandlerStatus::Yield);
    assert(missing_probe_result.fault == VmFault::Opcode007FRequiresButtonPressedProbe);
}

void test_fade_opcodes_preserve_native_parameter_adaptation_and_condition_vm() {
    std::vector<std::uint8_t> colour_bytes;
    append_u16(colour_bytes, 0x00C8);
    for (const std::int32_t value : {0x123, -1, 0xB4}) {
        colour_bytes.push_back(6);
        append_i32(colour_bytes, value);
    }
    ScriptVm colour_vm(ScriptSpace(std::move(colour_bytes)));
    bool colour_called = false;
    colour_vm.set_fade_set_colour_adapter([&](std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
        colour_called = true;
        assert(red == 0x23 && green == 0xFF && blue == 0xB4);
    });
    ScriptThread colour_thread;
    const ProcessResult colour_result = colour_vm.process_one_command(colour_thread);
    assert(colour_result.fault == VmFault::None && colour_result.status == NativeHandlerStatus::Continue);
    assert(colour_called && colour_thread.ip == 17);

    std::vector<std::uint8_t> fade_bytes;
    append_u16(fade_bytes, 0x00C9);
    fade_bytes.push_back(6);
    append_i32(fade_bytes, 2000);
    fade_bytes.push_back(6);
    append_i32(fade_bytes, 0x0000FFFF);
    ScriptVm fade_vm(ScriptSpace(std::move(fade_bytes)));
    bool fade_called = false;
    fade_vm.set_fade_start_adapter([&](float seconds, std::int16_t mode) {
        fade_called = true;
        assert(seconds == 2.0F && mode == -1);
    });
    ScriptThread fade_thread;
    const ProcessResult fade_result = fade_vm.process_one_command(fade_thread);
    assert(fade_result.fault == VmFault::None && fade_result.status == NativeHandlerStatus::Continue);
    assert(fade_called && fade_thread.ip == 12);

    std::vector<std::uint8_t> status_bytes;
    append_u16(status_bytes, 0x80CA); // raw NOT | fade status
    ScriptVm status_vm(ScriptSpace(std::move(status_bytes)));
    status_vm.set_fade_status_probe([] { return false; });
    ScriptThread status_thread;
    const ProcessResult status_result = status_vm.process_one_command(status_thread);
    assert(status_result.fault == VmFault::None && status_result.status == NativeHandlerStatus::Continue);
    assert(status_thread.not_flag && status_thread.condition_result);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x00CA);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode00CARequiresFadeStatusProbe);
}

void test_disable_pause_menu_normalizes_the_single_parameter_through_host() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0401);
    bytes.push_back(6); // INT32
    append_i32(bytes, -17); // Native branches only on zero/non-zero.
    append_u16(bytes, 0x0401);
    bytes.push_back(1); // INT_ZERO

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<bool> observed;
    vm.set_pause_menu_toggle_adapter([&](bool parameter_is_nonzero) {
        observed.push_back(parameter_is_nonzero);
    });
    ScriptThread thread;
    ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert((observed == std::vector<bool>{true, false}));

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0401);
    missing_adapter_bytes.push_back(1);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0401RequiresPauseMenuToggleAdapter);
}

void test_display_text_keeps_two_coordinate_payloads_and_key_separate() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0203);
    bytes.push_back(9); // raw binary32 coordinate payload
    append_i32(bytes, static_cast<std::int32_t>(0x3FC00000U)); // 1.5f
    bytes.push_back(9);
    append_i32(bytes, static_cast<std::int32_t>(0xC0200000U)); // -2.5f
    bytes.push_back(6); // separate text key / offset payload
    append_i32(bytes, 0x12345678);

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    bool adapter_called = false;
    vm.set_display_text_adapter([&](std::int32_t left, std::int32_t top, std::int32_t key) {
        adapter_called = true;
        assert(left == static_cast<std::int32_t>(0x3FC00000U));
        assert(top == static_cast<std::int32_t>(0xC0200000U));
        assert(key == 0x12345678);
    });
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(adapter_called && thread.ip == 17);

    // Exact three-parameter payload at MAIN.SCM 0x007BFF. In VCS plural
    // collection tag 0x00 is a raw zero coordinate, not the reLCS argument
    // terminator convention. The third int32 ends at the subsequent raw
    // 0x4843 data boundary.
    std::vector<std::uint8_t> main_payload;
    append_u16(main_payload, 0x0203);
    main_payload.push_back(0x00);
    main_payload.push_back(0x06);
    append_i32(main_payload, 0x00007BF3);
    main_payload.push_back(0x06);
    append_i32(main_payload, static_cast<std::int32_t>(0x0A01D000U));
    ScriptVm main_payload_vm(ScriptSpace(std::move(main_payload)));
    main_payload_vm.set_display_text_adapter([&](std::int32_t left, std::int32_t top, std::int32_t key) {
        assert(left == 0 && top == 0x7BF3);
        assert(static_cast<std::uint32_t>(key) == 0x0A01D000U);
    });
    ScriptThread main_payload_thread;
    const ProcessResult main_payload_result = main_payload_vm.process_one_command(main_payload_thread);
    assert(main_payload_result.fault == VmFault::None && main_payload_thread.ip == 13);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0203);
    missing_adapter_bytes.push_back(1);
    missing_adapter_bytes.push_back(1);
    missing_adapter_bytes.push_back(1);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0203RequiresDisplayTextAdapter);
}

void test_clear_help_is_a_no_parameter_host_boundary() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0268);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::size_t calls = 0;
    vm.set_clear_help_adapter([&] { ++calls; });
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 2 && calls == 1);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0268);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0268RequiresClearHelpAdapter);
}

void test_delete_vehicle_keeps_handle_and_native_context_flag_at_host_boundary() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0049);
    bytes.push_back(6); // INT32
    append_i32(bytes, 0x12345678);

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::pair<std::int32_t, bool>> observed;
    vm.set_vehicle_delete_adapter([&](std::int32_t vehicle_handle, bool native_context_flag) {
        observed.emplace_back(vehicle_handle, native_context_flag);
    });
    ScriptThread thread;
    thread.mission_launch_requested = true; // Portable analogue of native +0x20A.
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 7);
    assert((observed == std::vector<std::pair<std::int32_t, bool>>{{0x12345678, true}}));

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0049);
    missing_adapter_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0049RequiresVehicleDeleteAdapter);
}

void test_vehicle_release_mark_keeps_handle_and_separate_native_context_flag() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0113);
    bytes.push_back(6); // INT32
    append_i32(bytes, 0x12345678);

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::pair<std::int32_t, bool>> observed;
    vm.set_vehicle_release_mark_adapter([&](std::int32_t vehicle_handle, bool native_context_flag) {
        observed.emplace_back(vehicle_handle, native_context_flag);
    });
    ScriptThread thread;
    thread.mission_launch_requested = true; // Portable analogue of native +0x20A.
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 7);
    assert((observed == std::vector<std::pair<std::int32_t, bool>>{{0x12345678, true}}));

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0113);
    missing_adapter_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0113RequiresVehicleReleaseMarkAdapter);
}

void test_mission_streaming_boundary_consumes_its_one_selector_then_faults_loudly() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0289);
    bytes.push_back(6); // INT32
    append_i32(bytes, -2);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.status == NativeHandlerStatus::Yield);
    assert(result.fault == VmFault::Opcode0289RequiresMissionStreamingSubsystem);
    assert(thread.ip == 7); // handler decodes one selector before the boundary.
}

void test_create_pickup_preserves_model_object_ground_and_one_output_abi() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x013F);
    bytes.push_back(6); append_i32(bytes, -2);                         // negative model-object selector
    bytes.push_back(6); append_i32(bytes, 0x000001A2);                 // pickup type narrows to u8
    bytes.push_back(9); append_i32(bytes, 0x3FC00000);                 // x = 1.5f
    bytes.push_back(9); append_i32(bytes, 0x40100000);                 // y = 2.25f
    bytes.push_back(9); append_i32(bytes, static_cast<std::int32_t>(0xC2C80000U)); // z = -100.0f sentinel
    bytes.push_back(0x0D);                                             // StoreParameters local[0]
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    vm.set_pickup_model_object_resolver([](std::int32_t selector) -> std::optional<std::int32_t> {
        assert(selector == 2);
        return 411;
    });
    vm.set_pickup_ground_height_probe([](float x, float y) -> std::optional<float> {
        assert(x == 1.5F && y == 2.25F);
        return 32.0F;
    });
    vm.set_pickup_create_adapter([](std::int32_t model, std::uint8_t type, std::int32_t ammo, float x, float y, float z) {
        assert(model == 411 && type == 0xA2 && ammo == 0 && x == 1.5F && y == 2.25F && z == 32.5F);
        return 0x55AA;
    });
    ScriptThread thread;
    thread.locals.resize(1);
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.locals[0] == 0x55AA && thread.ip == 28);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x013F);
    for (int index = 0; index < 5; ++index) {
        missing_adapter_bytes.push_back(1); // raw zero; no model/ground sub-adapter required.
    }
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode013FRequiresPickupCreateAdapter);
    assert(missing_adapter_thread.ip == 7); // only five proven inputs were consumed.

    std::vector<std::uint8_t> ammo_bytes;
    append_u16(ammo_bytes, 0x01F9);
    ammo_bytes.push_back(6); append_i32(ammo_bytes, 88);               // model id
    ammo_bytes.push_back(6); append_i32(ammo_bytes, 0x00000103);       // low-byte pickup type
    ammo_bytes.push_back(6); append_i32(ammo_bytes, 900);              // ammo
    ammo_bytes.push_back(9); append_i32(ammo_bytes, 0x3F800000);       // x = 1.0f
    ammo_bytes.push_back(9); append_i32(ammo_bytes, 0x40000000);       // y = 2.0f
    ammo_bytes.push_back(9); append_i32(ammo_bytes, 0x40400000);       // z = 3.0f
    ammo_bytes.push_back(0x0D);
    ScriptVm ammo_vm(ScriptSpace(std::move(ammo_bytes)));
    ammo_vm.set_pickup_create_adapter([](std::int32_t model, std::uint8_t type, std::int32_t ammo, float x, float y, float z) {
        assert(model == 88 && type == 3 && ammo == 900 && x == 1.0F && y == 2.0F && z == 3.0F);
        return 0x66BB;
    });
    ScriptThread ammo_thread;
    ammo_thread.locals.resize(1);
    const ProcessResult ammo_result = ammo_vm.process_one_command(ammo_thread);
    assert(ammo_result.fault == VmFault::None && ammo_result.status == NativeHandlerStatus::Continue);
    assert(ammo_thread.locals[0] == 0x66BB && ammo_thread.ip == 33);
}

void test_terminate_this_script_preserves_scheduler_boundary_and_yield_status() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0023);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    bool adapter_called = false;
    vm.set_script_termination_adapter([&](ScriptVm&, ScriptThread& thread, bool was_mission_script) {
        adapter_called = true;
        assert(&thread != nullptr && was_mission_script);
    });
    ScriptThread thread;
    thread.is_mission_script = true;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Yield);
    assert(adapter_called && !thread.is_active && thread.terminated && thread.ip == 2);
    const ProcessResult inactive_result = vm.process(thread);
    assert(inactive_result.fault == VmFault::None && inactive_result.status == NativeHandlerStatus::Yield && inactive_result.executed == 0);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0023);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0023RequiresScriptTerminationAdapter);
    assert(missing_adapter_thread.is_active && !missing_adapter_thread.terminated);
}

void test_global_byte_store_preserves_offset_and_narrows_second_parameter() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x04E5);
    bytes.push_back(6); // INT32
    append_i32(bytes, 0x00001357);
    bytes.push_back(6); // INT32
    append_i32(bytes, 0x12345678);

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::pair<std::int32_t, std::uint8_t>> observed;
    vm.set_global_byte_store_adapter([&](std::int32_t slot_offset, std::uint8_t value) {
        observed.emplace_back(slot_offset, value);
    });
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 12);
    assert((observed == std::vector<std::pair<std::int32_t, std::uint8_t>>{{0x1357, 0x78}}));

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x04E5);
    missing_adapter_bytes.push_back(1); // INT_ZERO
    missing_adapter_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode04E5RequiresGlobalByteStoreAdapter);
}

void test_player_control_normalizes_the_second_parameter_through_host() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0107);
    bytes.push_back(6); // INT32
    append_i32(bytes, 3);
    bytes.push_back(6); // INT32
    append_i32(bytes, -1);
    append_u16(bytes, 0x0107);
    bytes.push_back(6); // INT32
    append_i32(bytes, 0);
    bytes.push_back(1); // INT_ZERO

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::pair<std::int32_t, bool>> observed;
    vm.set_player_control_adapter([&](std::int32_t player_selector, bool enabled) {
        observed.emplace_back(player_selector, enabled);
    });
    ScriptThread thread;
    assert(vm.process_one_command(thread).fault == VmFault::None);
    assert(vm.process_one_command(thread).fault == VmFault::None);
    assert((observed == std::vector<std::pair<std::int32_t, bool>>{{3, true}, {0, false}}));

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0107);
    missing_adapter_bytes.push_back(1); // INT_ZERO
    missing_adapter_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0107RequiresPlayerControlAdapter);
}

void test_character_coordinates_preserve_raw_floats_and_ground_sentinel() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0044);
    bytes.push_back(6); // handle
    append_i32(bytes, 0x2A);
    bytes.push_back(9); // x float
    append_i32(bytes, static_cast<std::int32_t>(0x3FA00000)); // 1.25f
    bytes.push_back(9); // y float
    append_i32(bytes, static_cast<std::int32_t>(0xC0200000)); // -2.5f
    bytes.push_back(9); // z float
    append_i32(bytes, static_cast<std::int32_t>(0xC2C80000)); // -100.0f

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::tuple<std::int32_t, std::int32_t, std::int32_t, std::int32_t, bool>> observed;
    vm.set_character_coordinate_adapter([&](std::int32_t handle, std::int32_t x, std::int32_t y,
                                             std::int32_t z, bool resolve_ground_height) {
        observed.emplace_back(handle, x, y, z, resolve_ground_height);
    });
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 22);
    assert(observed.size() == 1);
    assert(std::get<0>(observed[0]) == 0x2A && std::get<1>(observed[0]) == static_cast<std::int32_t>(0x3FA00000));
    assert(std::get<2>(observed[0]) == static_cast<std::int32_t>(0xC0200000));
    assert(std::get<3>(observed[0]) == static_cast<std::int32_t>(0xC2C80000));
    assert(std::get<4>(observed[0]));

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0044);
    for (int i = 0; i != 4; ++i)
        missing_adapter_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0044RequiresCharacterCoordinateAdapter);
}

void test_player_state_code_two_condition_uses_not_and_shared_reducer() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x00A5);
    bytes.push_back(6); // INT32
    append_i32(bytes, 7);
    append_u16(bytes, static_cast<std::uint16_t>(0x8000 | 0x00A5));
    bytes.push_back(1); // INT_ZERO

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::int32_t> selectors;
    vm.set_opcode_00a5_player_state_code_two_probe([&](ScriptVm&, ScriptThread&, std::int32_t player_selector) {
        selectors.push_back(player_selector);
        return true;
    });
    ScriptThread thread;
    assert(vm.process_one_command(thread).fault == VmFault::None);
    assert(thread.condition_result);
    assert(vm.process_one_command(thread).fault == VmFault::None);
    assert(!thread.condition_result);
    assert((selectors == std::vector<std::int32_t>{7, 0}));

    std::vector<std::uint8_t> missing_probe_bytes;
    append_u16(missing_probe_bytes, 0x00A5);
    missing_probe_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_probe_vm(ScriptSpace(std::move(missing_probe_bytes)));
    ScriptThread missing_probe_thread;
    const ProcessResult missing_probe_result = missing_probe_vm.process_one_command(missing_probe_thread);
    assert(missing_probe_result.status == NativeHandlerStatus::Yield);
    assert(missing_probe_result.fault == VmFault::Opcode00A5RequiresPlayerStateCodeTwoProbe);
}

void test_clear_prints_is_a_no_parameter_host_boundary() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0059);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::size_t calls = 0;
    vm.set_clear_prints_adapter([&] { ++calls; });
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 2 && calls == 1);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0059);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0059RequiresClearPrintsAdapter);
}

void test_character_dead_predicate_uses_not_and_shared_reducer() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x009F);
    bytes.push_back(6); // INT32
    append_i32(bytes, 42);
    append_u16(bytes, static_cast<std::uint16_t>(0x8000 | 0x009F));
    bytes.push_back(1); // INT_ZERO
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    std::vector<std::int32_t> handles;
    vm.set_opcode_009f_character_dead_probe([&](ScriptVm&, ScriptThread&, std::int32_t handle) {
        handles.push_back(handle);
        return true;
    });
    ScriptThread thread;
    assert(vm.process_one_command(thread).fault == VmFault::None && thread.condition_result);
    assert(vm.process_one_command(thread).fault == VmFault::None && !thread.condition_result);
    assert((handles == std::vector<std::int32_t>{42, 0}));

    std::vector<std::uint8_t> missing_probe_bytes;
    append_u16(missing_probe_bytes, 0x009F);
    missing_probe_bytes.push_back(1); // INT_ZERO
    ScriptVm missing_probe_vm(ScriptSpace(std::move(missing_probe_bytes)));
    ScriptThread missing_probe_thread;
    const ProcessResult missing_probe_result = missing_probe_vm.process_one_command(missing_probe_thread);
    assert(missing_probe_result.status == NativeHandlerStatus::Yield);
    assert(missing_probe_result.fault == VmFault::Opcode009FRequiresCharacterDeadProbe);
}

void test_get_game_timer_stores_raw_clock_dword_through_store_parameters() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x010E);
    bytes.push_back(0x0D); // one StoreParameters destination: local slot 0
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    vm.set_clock([] { return 0xFEDCBA98U; });
    ScriptThread thread;
    thread.locals.resize(1);
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 3 && static_cast<std::uint32_t>(thread.locals[0]) == 0xFEDCBA98U);
}

void test_get_pad_state_preserves_selector_boundary_and_stores_one_output() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0377);
    bytes.push_back(6); // selector
    append_i32(bytes, 2);
    bytes.push_back(0x0D); // StoreParameters destination
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    bool adapter_called = false;
    vm.set_pad_state_adapter([&](std::uint32_t selector) {
        adapter_called = true;
        assert(selector == 2);
        return -777;
    });
    ScriptThread thread;
    thread.locals.resize(1);
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(adapter_called && thread.locals[0] == -777 && thread.ip == 8);

    std::vector<std::uint8_t> out_of_range_bytes;
    append_u16(out_of_range_bytes, 0x0377);
    out_of_range_bytes.push_back(6);
    append_i32(out_of_range_bytes, -1); // unsigned selector is outside 0..44
    out_of_range_bytes.push_back(0x0D);
    ScriptVm out_of_range_vm(ScriptSpace(std::move(out_of_range_bytes)));
    ScriptThread out_of_range_thread;
    out_of_range_thread.locals.resize(1);
    const ProcessResult out_of_range_result = out_of_range_vm.process_one_command(out_of_range_thread);
    assert(out_of_range_result.fault == VmFault::None && out_of_range_result.status == NativeHandlerStatus::Continue);
    assert(out_of_range_thread.locals[0] == -1); // native leaves ScriptParams[0] unchanged

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0377);
    missing_adapter_bytes.push_back(1); // selector 0 is inside jump-table range
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0377RequiresPadStateAdapter);
}

void test_launch_mission_sets_only_the_proven_target_launch_flag() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0079);
    bytes.push_back(6);
    append_i32(bytes, 0x42);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread target;
    vm.set_mission_launch_adapter([&](ScriptVm&, std::int32_t mission_id) -> ScriptThread* {
        assert(mission_id == 0x42);
        return &target;
    });
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 7 && target.mission_launch_requested);

    std::vector<std::uint8_t> missing_adapter_bytes;
    append_u16(missing_adapter_bytes, 0x0079);
    missing_adapter_bytes.push_back(1);
    ScriptVm missing_adapter_vm(ScriptSpace(std::move(missing_adapter_bytes)));
    ScriptThread missing_adapter_thread;
    const ProcessResult missing_adapter_result = missing_adapter_vm.process_one_command(missing_adapter_thread);
    assert(missing_adapter_result.status == NativeHandlerStatus::Yield);
    assert(missing_adapter_result.fault == VmFault::Opcode0079RequiresMissionLaunchAdapter);
}

void test_script_name_uses_exact_eight_byte_copy_and_ascii_lowercase() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0238);
    bytes.push_back(10); // direct ScriptSpace string offset follows
    for (const char character : {'A', 'b', 'C', '_', '9', '9', '\0'})
        bytes.push_back(static_cast<std::uint8_t>(character));
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    const std::array<char, 8> expected{{'a', 'b', 'c', '_', '9', '9', '\0', '\0'}};
    assert(thread.script_name == expected && thread.ip == 10);

    std::vector<std::uint8_t> long_bytes;
    append_u16(long_bytes, 0x0238);
    long_bytes.push_back(10);
    for (const char character : {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', '\0'})
        long_bytes.push_back(static_cast<std::uint8_t>(character));
    ScriptVm long_vm(ScriptSpace(std::move(long_bytes)));
    ScriptThread long_thread;
    const ProcessResult long_result = long_vm.process_one_command(long_thread);
    const std::array<char, 8> long_expected{{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'}};
    assert(long_result.fault == VmFault::None && long_thread.script_name == long_expected);

    std::vector<std::uint8_t> null_source_bytes;
    append_u16(null_source_bytes, 0x0238);
    null_source_bytes.push_back(1);
    ScriptVm null_source_vm(ScriptSpace(std::move(null_source_bytes)));
    ScriptThread null_source_thread;
    const ProcessResult null_source_result = null_source_vm.process_one_command(null_source_thread);
    assert(null_source_result.status == NativeHandlerStatus::Yield);
    assert(null_source_result.fault == VmFault::InvalidScriptStringAddress);
}

void test_is_this_script_a_mission_reads_the_proven_thread_marker() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x04F2);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    thread.is_mission_script = true;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.condition_result && thread.ip == 2);

    std::vector<std::uint8_t> inverted_bytes;
    append_u16(inverted_bytes, 0x84F2);
    ScriptVm inverted_vm(ScriptSpace(std::move(inverted_bytes)));
    ScriptThread inverted_thread;
    inverted_thread.is_mission_script = true;
    inverted_thread.condition_state = 2;
    inverted_thread.condition_result = true;
    const ProcessResult inverted_result = inverted_vm.process_one_command(inverted_thread);
    assert(inverted_result.fault == VmFault::None && inverted_result.status == NativeHandlerStatus::Continue);
    assert(inverted_thread.not_flag && inverted_thread.condition_state == 1 && !inverted_thread.condition_result);
}

void test_gosub_pushes_tagless_return_frame_and_return_restores_ip() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0025);
    bytes.push_back(6); // INT32 script target
    append_i32(bytes, 10);
    bytes.resize(10, 0);
    append_u16(bytes, 0x0026); // RETURN at the GOSUB target

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 10 && thread.call_stack.size() == 1);
    assert(thread.call_stack.back().tag == 0 && thread.call_stack.back().call_header_ip == 7);

    result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 7 && thread.call_stack.empty());
}

void test_gosub_uses_the_confirmed_negative_main_script_target_convention() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0025);
    bytes.push_back(6); // INT32 script target
    append_i32(bytes, -5);
    bytes.resize(30, 0);

    // PS2 GOSUB resolves a negative target as main_script_size - target, so
    // -5 with a 20-byte main script transfers to script-space IP 25.
    ScriptVm vm(ScriptSpace(std::move(bytes), 20));
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 25 && thread.call_stack.size() == 1);
}

void test_gosub_file_consumes_two_parameters_and_pushes_tagless_return() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x01BA);
    bytes.push_back(6); // target parameter
    append_i32(bytes, 16);
    bytes.push_back(6); // native collects this second parameter but handler does not reread it
    append_i32(bytes, 0x12345678);
    bytes.resize(16, 0);
    append_u16(bytes, 0x0026); // RETURN at the target

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 16 && thread.call_stack.size() == 1);
    assert(thread.call_stack.back().tag == 0 && thread.call_stack.back().call_header_ip == 12);

    result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && result.status == NativeHandlerStatus::Continue);
    assert(thread.ip == 12 && thread.call_stack.empty());
}

void test_if_seeds_and_or_condition_groups() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0078);
    bytes.push_back(7); // INT8
    bytes.push_back(3); // AND group
    append_u16(bytes, 0x0078);
    bytes.push_back(7); // INT8
    bytes.push_back(9); // OR group
    append_u16(bytes, 0x0078);
    bytes.push_back(1); // INT_ZERO: clears state but does not change result

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && thread.condition_state == 3 && thread.condition_result);
    result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && thread.condition_state == 9 && !thread.condition_result);
    result = vm.process_one_command(thread);
    assert(result.fault == VmFault::None && thread.condition_state == 0 && !thread.condition_result);
}

void test_goto_if_false_and_return_true_false_use_condition_vm() {
    std::vector<std::uint8_t> branch_bytes;
    append_u16(branch_bytes, 0x0022);
    branch_bytes.push_back(6); // INT32 target
    append_i32(branch_bytes, 8);
    branch_bytes.resize(9, 0);
    ScriptVm branch_vm(ScriptSpace(std::move(branch_bytes)));
    ScriptThread false_thread;
    ProcessResult result = branch_vm.process_one_command(false_thread);
    assert(result.fault == VmFault::None && false_thread.ip == 8);
    ScriptThread true_thread;
    true_thread.condition_result = true;
    result = branch_vm.process_one_command(true_thread);
    assert(result.fault == VmFault::None && true_thread.ip == 7);

    for (const auto opcode_and_result : std::vector<std::pair<std::uint16_t, bool>>{
             {0x005E, true}, {0x005F, false}, {0x805F, true}}) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, 0x0025);
        bytes.push_back(6);
        append_i32(bytes, 10);
        bytes.resize(10, 0);
        append_u16(bytes, opcode_and_result.first);
        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        result = vm.process_one_command(thread); // GOSUB
        assert(result.fault == VmFault::None && thread.ip == 10);
        result = vm.process_one_command(thread); // RETURN_TRUE/FALSE
        assert(result.fault == VmFault::None && thread.ip == 7 && thread.call_stack.empty());
        assert(thread.condition_result == opcode_and_result.second);
    }
}

void test_call_and_call_not_preserve_the_confirmed_frame_tags() {
    for (const auto opcode_and_tag : std::vector<std::pair<std::uint16_t, std::uint32_t>>{
             {0x037B, 0x00800000}, {0x037A, 0x00C00000}}) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, opcode_and_tag.first);
        bytes.push_back(2); // input count
        bytes.push_back(0); // output count
        bytes.push_back(3); // locals offset
        bytes.push_back(6); // target: ARGUMENT_INT32
        append_i32(bytes, 16);
        bytes.push_back(8); // first input: ARGUMENT_INT16
        append_i16(bytes, 226);
        bytes.push_back(8); // second input: ARGUMENT_INT16
        append_i16(bytes, 183);
        bytes.push_back(0); // byte at target IP 16

        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals.resize(16);
        const ProcessResult result = vm.process_one_command(thread);

        assert(result.fault == VmFault::None);
        assert(result.status == NativeHandlerStatus::Continue);
        assert(thread.ip == 16);
        assert(thread.saved_ip == 2);
        assert(thread.locals_pointer == 3);
        assert(thread.locals[3] == 226);
        assert(thread.locals[4] == 183);
        assert(thread.call_stack.size() == 1);
        assert(thread.call_stack[0].tag == opcode_and_tag.second);
        assert(thread.call_stack[0].call_header_ip == 2);
        assert(thread.call_stack[0].return_parameter_bytes == 11);
    }
}

void test_call_return_copies_outputs_rolls_back_locals_and_applies_call_not() {
    for (const auto opcode_and_expected_condition : std::vector<std::pair<std::uint16_t, bool>>{
             {0x037B, false}, {0x037A, true}}) {
        std::vector<std::uint8_t> bytes;
        append_u16(bytes, opcode_and_expected_condition.first);
        bytes.push_back(2); // input count
        bytes.push_back(1); // output count
        bytes.push_back(3); // locals offset
        bytes.push_back(6); // target: ARGUMENT_INT32
        append_i32(bytes, 20);
        bytes.push_back(8); // first input: ARGUMENT_INT16
        append_i16(bytes, 226);
        bytes.push_back(8); // second input: ARGUMENT_INT16
        append_i16(bytes, 183);
        bytes.push_back(0x0D); // caller output destination: local[0]
        while (bytes.size() < 20)
            bytes.push_back(0);
        append_u16(bytes, 0x0026); // RETURN at callee target

        ScriptVm vm(ScriptSpace(std::move(bytes)));
        ScriptThread thread;
        thread.locals.resize(16);
        thread.condition_result = false;

        ProcessResult result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None && thread.ip == 20);
        assert(thread.locals_pointer == 3 && thread.call_stack.size() == 1);
        thread.locals[5] = 999; // current LP (3) + input count (2)

        result = vm.process_one_command(thread);
        assert(result.fault == VmFault::None);
        assert(result.status == NativeHandlerStatus::Continue);
        assert(result.last_opcode == 0x0026);
        assert(thread.call_stack.empty());
        assert(thread.locals_pointer == 0);
        assert(thread.locals[0] == 999);
        assert(thread.ip == 17); // Just after the single output destination.
        assert(thread.condition_result == opcode_and_expected_condition.second);
    }
}

void test_complete_immediate_decoder_and_peek_without_pc_advance() {
    std::vector<std::uint8_t> bytes{
        7, 0x80,                         // INT8 -> -128 (VCS PSP sign extends)
        8, 0xFF, 0xFF,                   // INT16 -> -1
        3, 0x7F,                         // FLOAT_1BYTE raw bits
        4, 0x34, 0x12,                   // FLOAT_2BYTES raw bits
        5, 0x56, 0x78, 0x9A,             // FLOAT_3BYTES raw bits
        9, 0x78, 0x56, 0x34, 0x12,       // FLOAT raw bits
        10, 'V', 'C', 'S', 0,            // SCRIPTSPACE string offset
    };
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    VmFault fault = VmFault::None;

    const auto int8 = vm.collect_next_parameter_without_increasing_pc(thread, 0, fault);
    assert(int8 && *int8 == -128 && thread.ip == 0 && fault == VmFault::None);
    const auto int16 = vm.collect_next_parameter_without_increasing_pc(thread, 2, fault);
    assert(int16 && *int16 == -1 && thread.ip == 0 && fault == VmFault::None);
    const auto f1 = vm.collect_next_parameter_without_increasing_pc(thread, 5, fault);
    assert(f1 && static_cast<std::uint32_t>(*f1) == 0x7F000000U && thread.ip == 0);
    const auto f2 = vm.collect_next_parameter_without_increasing_pc(thread, 7, fault);
    assert(f2 && static_cast<std::uint32_t>(*f2) == 0x12340000U && thread.ip == 0);
    const auto f3 = vm.collect_next_parameter_without_increasing_pc(thread, 10, fault);
    assert(f3 && static_cast<std::uint32_t>(*f3) == 0x9A785600U && thread.ip == 0);
    const auto f4 = vm.collect_next_parameter_without_increasing_pc(thread, 14, fault);
    assert(f4 && static_cast<std::uint32_t>(*f4) == 0x12345678U && thread.ip == 0);
    thread.ip = 19;
    std::vector<std::int32_t> string_parameter;
    assert(vm.collect_parameters(thread, 1, string_parameter, fault));
    assert(string_parameter.size() == 1 && string_parameter[0] == 20 && thread.ip == 24 && fault == VmFault::None);
}

void test_vcs_plural_tag_zero_differs_from_standalone_decoder() {
    std::vector<std::uint8_t> bytes{0};
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    thread.locals.resize(0x5E);
    thread.locals[0x5D] = 91;
    VmFault fault = VmFault::None;

    const auto standalone = vm.collect_next_parameter_without_increasing_pc(thread, 0, fault);
    assert(standalone && *standalone == 91 && fault == VmFault::None);

    const auto consuming = vm.read_parameter(thread, fault);
    assert(consuming && *consuming == 0 && thread.ip == 1 && fault == VmFault::None);

    thread.ip = 0;
    std::vector<std::int32_t> parameters;
    assert(vm.collect_parameters(thread, 1, parameters, fault));
    assert(fault == VmFault::None);
    assert((parameters == std::vector<std::int32_t>{0}));
    assert(thread.ip == 1);
}

void test_variable_resolver_and_store_parameters() {
    std::vector<std::uint8_t> bytes(1024);
    constexpr std::uint32_t kParameterIp = 512;
    // local[locals_pointer + 0], local-array(base 0, index local[5], size 3),
    // global block 0/index 2, global-array block 0/index 3/index local[5]/size 4.
    bytes[kParameterIp + 0] = 0x0D;
    bytes[kParameterIp + 1] = 0x6D;
    bytes[kParameterIp + 2] = 5;
    bytes[kParameterIp + 3] = 3;
    bytes[kParameterIp + 4] = 0xCD;
    bytes[kParameterIp + 5] = 2;
    bytes[kParameterIp + 6] = 0xE6;
    bytes[kParameterIp + 7] = 3;
    bytes[kParameterIp + 8] = 5;
    bytes[kParameterIp + 9] = 4;

    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    thread.locals.resize(128);
    thread.locals_pointer = 10;
    thread.locals[10] = 111;
    thread.locals[15] = 2;
    thread.locals[12] = 222;
    assert(vm.script_space().write_i32_at(8, 333));
    assert(vm.script_space().write_i32_at(20, 444));

    VmFault fault = VmFault::None;
    const auto local = vm.collect_next_parameter_without_increasing_pc(thread, kParameterIp, fault);
    const auto local_array = vm.collect_next_parameter_without_increasing_pc(thread, kParameterIp + 1, fault);
    const auto global = vm.collect_next_parameter_without_increasing_pc(thread, kParameterIp + 4, fault);
    const auto global_array = vm.collect_next_parameter_without_increasing_pc(thread, kParameterIp + 6, fault);
    assert(local && *local == 111);
    assert(local_array && *local_array == 222);
    assert(global && *global == 333);
    assert(global_array && *global_array == 444);
    assert(thread.ip == 0 && fault == VmFault::None);

    // StoreParameters consumes variable destinations from the thread's PC.
    thread.ip = kParameterIp;
    assert(vm.store_parameters(thread, {700, 800}, fault));
    assert(thread.ip == kParameterIp + 4);
    assert(thread.locals[10] == 700);
    assert(thread.locals[12] == 800);

    thread.ip = kParameterIp + 4;
    assert(vm.store_parameters(thread, {900, 1000}, fault));
    std::int32_t global_value = 0;
    std::int32_t global_array_value = 0;
    assert(vm.script_space().read_i32_at(8, global_value));
    assert(vm.script_space().read_i32_at(20, global_array_value));
    assert(global_value == 900 && global_array_value == 1000);
}

void test_invalid_array_index_faults_loudly() {
    std::vector<std::uint8_t> bytes{0x6D, 0, 2};
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    thread.locals.resize(106);
    thread.locals[0] = 2; // Equal to size: VCS PSP rejects it.
    VmFault fault = VmFault::None;

    const auto value = vm.read_parameter(thread, fault);
    assert(!value);
    assert(fault == VmFault::InvalidArrayIndex);
}

void test_055a_uses_host_probe_and_exact_condition_reducer() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x055A);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    ProcessResult result = vm.process_one_command(thread);
    assert(result.fault == VmFault::Opcode055ARequiresConditionProbe);

    std::vector<std::uint8_t> adapted_bytes;
    append_u16(adapted_bytes, 0x055A);
    ScriptVm adapted_vm(ScriptSpace(std::move(adapted_bytes)));
    adapted_vm.set_opcode_055a_condition_probe([](ScriptVm&, ScriptThread&) { return true; });
    ScriptThread adapted_thread;
    adapted_thread.not_flag = false;
    result = adapted_vm.process_one_command(adapted_thread);
    assert(result.fault == VmFault::None);
    assert(result.status == NativeHandlerStatus::Continue);
    assert(adapted_thread.condition_state == 0);
    assert(adapted_thread.condition_result);

    // The reducer, not the host, applies high-opcode NOT.
    adapted_thread.not_flag = true;
    adapted_vm.reduce_condition_result(adapted_thread, true);
    assert(!adapted_thread.condition_result);

    // States 1..8 aggregate with AND and only decrement their counter.
    adapted_thread.not_flag = false;
    adapted_thread.condition_state = 3;
    adapted_thread.condition_result = true;
    adapted_vm.reduce_condition_result(adapted_thread, false);
    assert(adapted_thread.condition_state == 2);
    assert(!adapted_thread.condition_result);

    // State 9 begins the OR path; after decrement the native < 0x15 reset
    // makes it terminal. State 22 proves that large OR counters persist.
    adapted_thread.condition_state = 9;
    adapted_thread.condition_result = false;
    adapted_vm.reduce_condition_result(adapted_thread, true);
    assert(adapted_thread.condition_state == 0);
    assert(adapted_thread.condition_result);
    adapted_thread.condition_state = 22;
    adapted_thread.condition_result = false;
    adapted_vm.reduce_condition_result(adapted_thread, false);
    assert(adapted_thread.condition_state == 21);
    assert(!adapted_thread.condition_result);
}

void test_set_unique_jumps_total_requires_host_and_preserves_raw_int32() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x01E7);
    bytes.push_back(6); // INT32
    append_i32(bytes, 99);

    ScriptVm missing_vm{ScriptSpace(bytes)};
    ScriptThread missing_thread;
    ProcessResult result = missing_vm.process_one_command(missing_thread);
    assert(result.status == NativeHandlerStatus::Yield);
    assert(result.fault == VmFault::Opcode01E7RequiresUniqueJumpsTotalAdapter);
    assert(missing_thread.ip == 7);

    ScriptVm vm{ScriptSpace(std::move(bytes))};
    std::int32_t observed = 0;
    vm.set_unique_jumps_total_adapter([&](std::int32_t total) { observed = total; });
    ScriptThread thread;
    result = vm.process_one_command(thread);
    assert(result.status == NativeHandlerStatus::Continue && result.fault == VmFault::None);
    assert(observed == 99 && thread.ip == 7);
}

void test_unknown_slots_are_faults_not_false_yields() {
    std::vector<std::uint8_t> bytes;
    append_u16(bytes, 0x0003);
    ScriptVm vm(ScriptSpace(std::move(bytes)));
    ScriptThread thread;
    const ProcessResult result = vm.process_one_command(thread);
    assert(result.status == NativeHandlerStatus::Yield);
    assert(result.fault == VmFault::UnsupportedOpcode);
}

void test_script_space_bounded_byte_staging() {
    ScriptSpace space(std::vector<std::uint8_t>(8, 0));
    assert(space.write_bytes_at(2, {0x11, 0x22, 0x33}));
    std::uint32_t ip = 2;
    std::int8_t value = 0;
    assert(space.read_i8(ip, value) && value == 0x11);
    assert(space.read_i8(ip, value) && value == 0x22);
    assert(space.read_i8(ip, value) && value == 0x33);
    assert(!space.write_bytes_at(6, {0x44, 0x55, 0x66}));
}

} // namespace

int main() {
    test_dispatcher_strips_not_bit_and_nop_continues();
    test_wait_yields_and_uses_the_immediate_delay();
    test_goto_sets_ip_and_continues();
    test_start_new_script_clamps_index_and_copies_96_locals_through_host();
    test_numeric_conversions_and_absolute_value_handlers();
    test_timed_float_arithmetic_uses_host_time_step();
    test_assign_aliases_store_raw_parameter_dwords();
    test_set_var_text_label_stores_the_collected_scriptspace_string_offset();
    test_add_value_to_int_var_uses_modulo_32_bit_math();
    test_remaining_scalar_arithmetic_handlers();
    test_integer_condition_handlers_use_signed_predicates_and_condition_reducer();
    test_float_condition_handlers_preserve_mips_unordered_and_raw_equality_rules();
    test_is_char_still_alive_uses_host_predicate_and_exact_condition_vm();
    test_is_button_pressed_truncates_two_inputs_and_uses_condition_vm();
    test_fade_opcodes_preserve_native_parameter_adaptation_and_condition_vm();
    test_disable_pause_menu_normalizes_the_single_parameter_through_host();
    test_display_text_keeps_two_coordinate_payloads_and_key_separate();
    test_clear_help_is_a_no_parameter_host_boundary();
    test_delete_vehicle_keeps_handle_and_native_context_flag_at_host_boundary();
    test_vehicle_release_mark_keeps_handle_and_separate_native_context_flag();
    test_mission_streaming_boundary_consumes_its_one_selector_then_faults_loudly();
    test_create_pickup_preserves_model_object_ground_and_one_output_abi();
    test_set_unique_jumps_total_requires_host_and_preserves_raw_int32();
    test_terminate_this_script_preserves_scheduler_boundary_and_yield_status();
    test_global_byte_store_preserves_offset_and_narrows_second_parameter();
    test_player_control_normalizes_the_second_parameter_through_host();
    test_character_coordinates_preserve_raw_floats_and_ground_sentinel();
    test_player_state_code_two_condition_uses_not_and_shared_reducer();
    test_clear_prints_is_a_no_parameter_host_boundary();
    test_character_dead_predicate_uses_not_and_shared_reducer();
    test_get_game_timer_stores_raw_clock_dword_through_store_parameters();
    test_get_pad_state_preserves_selector_boundary_and_stores_one_output();
    test_launch_mission_sets_only_the_proven_target_launch_flag();
    test_script_name_uses_exact_eight_byte_copy_and_ascii_lowercase();
    test_is_this_script_a_mission_reads_the_proven_thread_marker();
    test_gosub_pushes_tagless_return_frame_and_return_restores_ip();
    test_gosub_uses_the_confirmed_negative_main_script_target_convention();
    test_gosub_file_consumes_two_parameters_and_pushes_tagless_return();
    test_if_seeds_and_or_condition_groups();
    test_goto_if_false_and_return_true_false_use_condition_vm();
    test_call_and_call_not_preserve_the_confirmed_frame_tags();
    test_call_return_copies_outputs_rolls_back_locals_and_applies_call_not();
    test_complete_immediate_decoder_and_peek_without_pc_advance();
    test_vcs_plural_tag_zero_differs_from_standalone_decoder();
    test_variable_resolver_and_store_parameters();
    test_invalid_array_index_faults_loudly();
    test_055a_uses_host_probe_and_exact_condition_reducer();
    test_unknown_slots_are_faults_not_false_yields();
    test_script_space_bounded_byte_staging();
    std::cout << "vcs_vm_core tests passed\n";
}
