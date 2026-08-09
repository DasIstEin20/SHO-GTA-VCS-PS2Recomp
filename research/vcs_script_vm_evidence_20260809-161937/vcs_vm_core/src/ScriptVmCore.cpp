#include "vcs_vm/ScriptVmCore.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

namespace vcs::vm {
namespace {

constexpr std::uint16_t kOpcodeNop = 0x0000;
constexpr std::uint16_t kOpcodeWait = 0x0001;
constexpr std::uint16_t kOpcodeGoto = 0x0002;
constexpr std::uint16_t kOpcodeSetVarInt = 0x0004;
constexpr std::uint16_t kOpcodeSetVarFloat = 0x0005;
constexpr std::uint16_t kOpcodeSetVarTextLabel = 0x0006;
constexpr std::uint16_t kOpcodeAddValueToIntVar = 0x0007;
constexpr std::uint16_t kOpcodeAddValueToFloatVar = 0x0008;
constexpr std::uint16_t kOpcodeSubValueFromIntVar = 0x0009;
constexpr std::uint16_t kOpcodeSubValueFromFloatVar = 0x000A;
constexpr std::uint16_t kOpcodeMultiplyIntVarByValue = 0x000B;
constexpr std::uint16_t kOpcodeMultiplyFloatVarByValue = 0x000C;
constexpr std::uint16_t kOpcodeDivideIntVarByValue = 0x000D;
constexpr std::uint16_t kOpcodeDivideFloatVarByValue = 0x000E;
constexpr std::uint16_t kOpcodeIsIntVarGreaterThanNumber = 0x000F;
constexpr std::uint16_t kOpcodeIsNumberGreaterThanIntVar = 0x0010;
constexpr std::uint16_t kOpcodeIsIntVarGreaterThanIntVar = 0x0011;
constexpr std::uint16_t kOpcodeIsFloatVarGreaterThanNumber = 0x0012;
constexpr std::uint16_t kOpcodeIsNumberGreaterThanFloatVar = 0x0013;
constexpr std::uint16_t kOpcodeIsFloatVarGreaterThanFloatVar = 0x0014;
constexpr std::uint16_t kOpcodeIsIntVarGreaterOrEqualToNumber = 0x0015;
constexpr std::uint16_t kOpcodeIsNumberGreaterOrEqualToIntVar = 0x0016;
constexpr std::uint16_t kOpcodeIsIntVarGreaterOrEqualToIntVar = 0x0017;
constexpr std::uint16_t kOpcodeIsFloatVarGreaterOrEqualToNumber = 0x0018;
constexpr std::uint16_t kOpcodeIsNumberGreaterOrEqualToFloatVar = 0x0019;
constexpr std::uint16_t kOpcodeIsFloatVarGreaterOrEqualToFloatVar = 0x001A;
constexpr std::uint16_t kOpcodeIsIntVarEqualToNumber = 0x001B;
constexpr std::uint16_t kOpcodeIsIntVarEqualToIntVar = 0x001C;
constexpr std::uint16_t kOpcodeIsFloatVarEqualToNumber = 0x001D;
constexpr std::uint16_t kOpcodeIsFloatVarEqualToFloatVar = 0x001E;
constexpr std::uint16_t kOpcodeGotoIfFalse = 0x0022;
constexpr std::uint16_t kOpcodeTerminateThisScript = 0x0023;
constexpr std::uint16_t kOpcodeStartNewScript = 0x0024;
constexpr std::uint16_t kOpcodeGosub = 0x0025;
constexpr std::uint16_t kOpcodeReturn = 0x0026;
constexpr std::uint16_t kOpcodeAddIntVarToIntVar = 0x0029;
// PSP and PS2 table entries 002A..0030 point to the exact same native
// arithmetic targets as 0008..000E. The identifiers deliberately state only
// the dispatch fact, not unproved public SCM source-kind spellings.
constexpr std::uint16_t kOpcodeFloatAddDispatchAlias = 0x002A;
constexpr std::uint16_t kOpcodeIntegerSubtractDispatchAlias = 0x002B;
constexpr std::uint16_t kOpcodeFloatSubtractDispatchAlias = 0x002C;
constexpr std::uint16_t kOpcodeIntegerMultiplyDispatchAlias = 0x002D;
constexpr std::uint16_t kOpcodeFloatMultiplyDispatchAlias = 0x002E;
constexpr std::uint16_t kOpcodeIntegerDivideDispatchAlias = 0x002F;
constexpr std::uint16_t kOpcodeFloatDivideDispatchAlias = 0x0030;
constexpr std::uint16_t kOpcodeAddTimedValueToFloatVar = 0x0031;
constexpr std::uint16_t kOpcodeAddTimedFloatVarToFloatVar = 0x0032;
constexpr std::uint16_t kOpcodeSubTimedValueFromFloatVar = 0x0033;
constexpr std::uint16_t kOpcodeSubTimedFloatVarFromFloatVar = 0x0034;
constexpr std::uint16_t kOpcodeSetVarIntToVarInt = 0x0035;
constexpr std::uint16_t kOpcodeSetVarFloatToVarFloat = 0x0036;
constexpr std::uint16_t kOpcodeSetVarTextLabelToVarTextLabel = 0x0037;
constexpr std::uint16_t kOpcodeCSetVarIntToVarFloat = 0x0038;
constexpr std::uint16_t kOpcodeCSetVarFloatToVarInt = 0x0039;
constexpr std::uint16_t kOpcodeAbsInt = 0x003A;
constexpr std::uint16_t kOpcodeAbsFloat = 0x003B;
constexpr std::uint16_t kOpcodeReturnTrue = 0x005E;
constexpr std::uint16_t kOpcodeReturnFalse = 0x005F;
constexpr std::uint16_t kOpcodeClearPrints = 0x0059;
constexpr std::uint16_t kOpcodeIsCharStillAlive = 0x0045;
constexpr std::uint16_t kOpcodeSetCharacterCoordinates = 0x0044;
constexpr std::uint16_t kOpcodeDeleteVehicle = 0x0049;
constexpr std::uint16_t kOpcodeVehicleReleaseMark = 0x0113;
constexpr std::uint16_t kOpcodeCreatePickup = 0x013F;
constexpr std::uint16_t kOpcodeCreatePickupWithAmmo = 0x01F9;
constexpr std::uint16_t kOpcodeSetUniqueJumpsTotal = 0x01E7;
constexpr std::uint16_t kOpcodeIf = 0x0078;
constexpr std::uint16_t kOpcodeLaunchMission = 0x0079;
constexpr std::uint16_t kOpcodeIsButtonPressed = 0x007F;
constexpr std::uint16_t kOpcodePlayerStateCodeTwoCondition = 0x00A5;
constexpr std::uint16_t kOpcodeCharacterDeadCondition = 0x009F;
constexpr std::uint16_t kOpcodeSetFadingColour = 0x00C8;
constexpr std::uint16_t kOpcodeDoFade = 0x00C9;
constexpr std::uint16_t kOpcodeGetFadingStatus = 0x00CA;
constexpr std::uint16_t kOpcodeGetGameTimer = 0x010E;
constexpr std::uint16_t kOpcodePlayerControl = 0x0107;
constexpr std::uint16_t kOpcodeDisplayText = 0x0203;
constexpr std::uint16_t kOpcodeScriptName = 0x0238;
constexpr std::uint16_t kOpcodeClearHelp = 0x0268;
constexpr std::uint16_t kOpcodeMissionStreamingBoundary = 0x0289;
constexpr std::uint16_t kOpcodeGetPadState = 0x0377;
constexpr std::uint16_t kOpcodeDisablePauseMenu = 0x0401;
constexpr std::uint16_t kOpcodeGosubFile = 0x01BA;
constexpr std::uint16_t kOpcodeSetVarIntToConstant = 0x02E2;
constexpr std::uint16_t kOpcodeIsIntVarEqualToConstant = 0x02DB;
constexpr std::uint16_t kOpcodeCallNot = 0x037A;
constexpr std::uint16_t kOpcodeCall = 0x037B;
constexpr std::uint16_t kOpcodeIsThisScriptAMission = 0x04F2;
constexpr std::uint16_t kOpcodeGlobalByteStore = 0x04E5;
constexpr std::uint16_t kOpcode055A = 0x055A;

constexpr std::uint32_t kCallTag = 0x00800000;
constexpr std::uint32_t kCallNotTag = 0x00C00000;

constexpr std::uint8_t kArgumentIntZero = 1;
constexpr std::uint8_t kArgumentFloatZero = 2;
constexpr std::uint8_t kArgumentFloat1Byte = 3;
constexpr std::uint8_t kArgumentFloat2Bytes = 4;
constexpr std::uint8_t kArgumentFloat3Bytes = 5;
constexpr std::uint8_t kArgumentInt32 = 6;
constexpr std::uint8_t kArgumentInt8 = 7;
constexpr std::uint8_t kArgumentInt16 = 8;
constexpr std::uint8_t kArgumentFloat = 9;
constexpr std::uint8_t kArgumentString = 10;
constexpr std::uint8_t kArgumentVcsPluralZero = 0;

// These byte ranges are directly visible in VCS PSP FUN_0005DA7C. The
// low-range tail is deliberately named by shape, not as a timer contract:
// VCS maps the resolver's low range 0x00..0x0C to local slots 0x5D..0x69.
// Its plural collector separately gives tag 0x00 a zero-value case before the
// standalone decoder/resolver path is reached.
constexpr std::uint8_t kArgumentLocal = 0x0D;
constexpr std::uint8_t kArgumentLocalArray = 0x6D;
constexpr std::uint8_t kArgumentGlobal = 0xCD;
constexpr std::uint8_t kArgumentGlobalArray = 0xE6;
constexpr std::uint32_t kLowVariableTailBase = 0x5D;

float float_from_raw_parameter(std::int32_t value) {
    // The portable core's supported host is the PC IEEE-754 binary32 model.
    // memcpy preserves the script parameter's native bit pattern without
    // aliasing UB; MIPS c.le.s/c.lt.s semantics are then expressed below.
    static_assert(sizeof(float) == sizeof(std::uint32_t), "VCS float parameters require binary32 storage");
    const std::uint32_t bits = static_cast<std::uint32_t>(value);
    float result = 0.0F;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

std::int32_t raw_parameter_from_float(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return static_cast<std::int32_t>(bits);
}

} // namespace

ScriptSpace::ScriptSpace(std::vector<std::uint8_t> bytes, std::uint32_t main_script_size)
    : bytes_(std::move(bytes)),
      main_script_size_(main_script_size == 0 ? static_cast<std::uint32_t>(bytes_.size()) : main_script_size) {}

std::uint32_t ScriptSpace::size() const {
    return static_cast<std::uint32_t>(bytes_.size());
}

std::uint32_t ScriptSpace::main_script_size() const {
    return main_script_size_;
}

bool ScriptSpace::is_valid_ip(std::uint32_t ip) const {
    return ip < bytes_.size();
}

bool ScriptSpace::read_u8(std::uint32_t& ip, std::uint8_t& value) const {
    if (!is_valid_ip(ip))
        return false;
    value = bytes_[ip++];
    return true;
}

bool ScriptSpace::read_i8(std::uint32_t& ip, std::int8_t& value) const {
    std::uint8_t byte = 0;
    if (!read_u8(ip, byte))
        return false;
    value = static_cast<std::int8_t>(byte);
    return true;
}

bool ScriptSpace::read_i16(std::uint32_t& ip, std::int16_t& value) const {
    std::uint8_t lo = 0;
    std::uint8_t hi = 0;
    if (!read_u8(ip, lo) || !read_u8(ip, hi))
        return false;
    const std::uint16_t raw = static_cast<std::uint16_t>(lo) | (static_cast<std::uint16_t>(hi) << 8U);
    value = static_cast<std::int16_t>(raw);
    return true;
}

bool ScriptSpace::read_i32(std::uint32_t& ip, std::int32_t& value) const {
    std::uint8_t bytes[4]{};
    for (std::uint8_t& byte : bytes) {
        if (!read_u8(ip, byte))
            return false;
    }
    const std::uint32_t raw = static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[2]) << 16U) |
        (static_cast<std::uint32_t>(bytes[3]) << 24U);
    value = static_cast<std::int32_t>(raw);
    return true;
}

bool ScriptSpace::read_i32_at(std::uint32_t address, std::int32_t& value) const {
    if (address > bytes_.size() || bytes_.size() - address < 4)
        return false;
    const std::uint32_t raw = static_cast<std::uint32_t>(bytes_[address]) |
        (static_cast<std::uint32_t>(bytes_[address + 1]) << 8U) |
        (static_cast<std::uint32_t>(bytes_[address + 2]) << 16U) |
        (static_cast<std::uint32_t>(bytes_[address + 3]) << 24U);
    value = static_cast<std::int32_t>(raw);
    return true;
}

bool ScriptSpace::write_i32_at(std::uint32_t address, std::int32_t value) {
    if (address > bytes_.size() || bytes_.size() - address < 4)
        return false;
    const std::uint32_t raw = static_cast<std::uint32_t>(value);
    bytes_[address] = static_cast<std::uint8_t>(raw);
    bytes_[address + 1] = static_cast<std::uint8_t>(raw >> 8U);
    bytes_[address + 2] = static_cast<std::uint8_t>(raw >> 16U);
    bytes_[address + 3] = static_cast<std::uint8_t>(raw >> 24U);
    return true;
}

bool ScriptSpace::write_bytes_at(std::uint32_t address, const std::vector<std::uint8_t>& bytes) {
    if (address > bytes_.size() || bytes.size() > bytes_.size() - address)
        return false;
    std::copy(bytes.begin(), bytes.end(), bytes_.begin() + address);
    return true;
}

ScriptVm::ScriptVm(ScriptSpace script_space, std::size_t max_call_depth)
    : script_space_(std::move(script_space)),
      clock_([] { return 0U; }),
      max_call_depth_(max_call_depth) {
    register_handler(kOpcodeNop, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_nop(thread); });
    register_handler(kOpcodeWait, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_wait(thread); });
    register_handler(kOpcodeGoto, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_goto(thread); });
    register_handler(kOpcodeSetVarInt, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeSetVarFloat, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeSetVarTextLabel, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeAddValueToIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeAddValueToFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeSubValueFromIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeSubValueFromFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeMultiplyIntVarByValue, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeMultiplyFloatVarByValue, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeDivideIntVarByValue, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeDivideFloatVarByValue, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeIsIntVarGreaterThanNumber, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsNumberGreaterThanIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsIntVarGreaterThanIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsFloatVarGreaterThanNumber, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_condition(thread, opcode); });
    register_handler(kOpcodeIsNumberGreaterThanFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_condition(thread, opcode); });
    register_handler(kOpcodeIsFloatVarGreaterThanFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_condition(thread, opcode); });
    register_handler(kOpcodeIsIntVarGreaterOrEqualToNumber, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsNumberGreaterOrEqualToIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsIntVarGreaterOrEqualToIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsFloatVarGreaterOrEqualToNumber, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_condition(thread, opcode); });
    register_handler(kOpcodeIsNumberGreaterOrEqualToFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_condition(thread, opcode); });
    register_handler(kOpcodeIsFloatVarGreaterOrEqualToFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_condition(thread, opcode); });
    register_handler(kOpcodeIsIntVarEqualToNumber, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsIntVarEqualToIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsFloatVarEqualToNumber, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsFloatVarEqualToFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeIsCharStillAlive, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_0045_is_char_still_alive(thread); });
    register_handler(kOpcodePlayerStateCodeTwoCondition, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_00a5_player_state_code_two_condition(thread); });
    register_handler(kOpcodeClearPrints, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_clear_prints(thread); });
    register_handler(kOpcodeCharacterDeadCondition, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_009f_character_dead_condition(thread); });
    register_handler(kOpcodeLaunchMission, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_launch_mission(thread); });
    register_handler(kOpcodeIsButtonPressed, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_007f_is_button_pressed(thread); });
    register_handler(kOpcodeSetFadingColour, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_set_fading_colour(thread); });
    register_handler(kOpcodeDoFade, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_do_fade(thread); });
    register_handler(kOpcodeGetFadingStatus, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_get_fading_status(thread); });
    register_handler(kOpcodeDeleteVehicle, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_delete_vehicle(thread); });
    register_handler(kOpcodeVehicleReleaseMark, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_vehicle_release_mark(thread); });
    register_handler(kOpcodeCreatePickup, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_create_pickup(thread, false); });
    register_handler(kOpcodeCreatePickupWithAmmo, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_create_pickup(thread, true); });
    register_handler(kOpcodeSetUniqueJumpsTotal, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_set_unique_jumps_total(thread); });
    register_handler(kOpcodeGlobalByteStore, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_global_byte_store(thread); });
    register_handler(kOpcodePlayerControl, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_player_control(thread); });
    register_handler(kOpcodeSetCharacterCoordinates, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_character_coordinate_placement(thread); });
    register_handler(kOpcodeDisplayText, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_display_text(thread); });
    register_handler(kOpcodeClearHelp, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_clear_help(thread); });
    register_handler(kOpcodeDisablePauseMenu, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_pause_menu_toggle(thread); });
    register_handler(kOpcodeGetGameTimer, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_get_game_timer(thread); });
    register_handler(kOpcodeScriptName, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_script_name(thread); });
    register_handler(kOpcodeMissionStreamingBoundary, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_mission_streaming_boundary(thread); });
    register_handler(kOpcodeGetPadState, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_get_pad_state(thread); });
    register_handler(kOpcodeGotoIfFalse, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_goto_if_false(thread); });
    register_handler(kOpcodeTerminateThisScript, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_terminate_this_script(thread); });
    register_handler(kOpcodeStartNewScript, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_start_new_script(thread); });
    register_handler(kOpcodeGosub, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_gosub(thread); });
    register_handler(kOpcodeReturn, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_return(thread); });
    register_handler(kOpcodeAddIntVarToIntVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeFloatAddDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeIntegerSubtractDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeFloatSubtractDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeIntegerMultiplyDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeFloatMultiplyDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeIntegerDivideDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integer_arithmetic(thread, opcode); });
    register_handler(kOpcodeFloatDivideDispatchAlias, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeAddTimedValueToFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_timed_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeAddTimedFloatVarToFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_timed_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeSubTimedValueFromFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_timed_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeSubTimedFloatVarFromFloatVar, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_timed_float_arithmetic(thread, opcode); });
    register_handler(kOpcodeSetVarIntToVarInt, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeSetVarFloatToVarFloat, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeSetVarTextLabelToVarTextLabel, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeCSetVarIntToVarFloat, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_numeric_conversion(thread, opcode); });
    register_handler(kOpcodeCSetVarFloatToVarInt, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_numeric_conversion(thread, opcode); });
    register_handler(kOpcodeAbsInt, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_absolute_value(thread, opcode); });
    register_handler(kOpcodeAbsFloat, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_absolute_value(thread, opcode); });
    register_handler(kOpcodeReturnTrue, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_return_condition(thread, true); });
    register_handler(kOpcodeReturnFalse, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_return_condition(thread, false); });
    register_handler(kOpcodeIf, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_if(thread); });
    register_handler(kOpcodeGosubFile, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_gosub_file(thread); });
    register_handler(kOpcodeSetVarIntToConstant, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_assign(thread); });
    register_handler(kOpcodeIsIntVarEqualToConstant, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t opcode) { return vm.handle_integral_or_raw_equality_condition(thread, opcode); });
    register_handler(kOpcodeCallNot, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_call(thread, true); });
    register_handler(kOpcodeCall, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_call(thread, false); });
    register_handler(kOpcodeIsThisScriptAMission, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_is_this_script_a_mission(thread); });
    register_handler(kOpcode055A, [](ScriptVm& vm, ScriptThread& thread, std::uint16_t) { return vm.handle_055a(thread); });
}

ScriptSpace& ScriptVm::script_space() {
    return script_space_;
}

const ScriptSpace& ScriptVm::script_space() const {
    return script_space_;
}

void ScriptVm::set_clock(std::function<std::uint32_t()> clock) {
    clock_ = std::move(clock);
}

void ScriptVm::set_opcode_055a_condition_probe(Opcode055AConditionProbe probe) {
    opcode_055a_condition_probe_ = std::move(probe);
}

void ScriptVm::set_start_new_script_adapter(StartNewScriptAdapter adapter) {
    start_new_script_adapter_ = std::move(adapter);
}

void ScriptVm::set_time_step_provider(TimeStepProvider provider) {
    time_step_provider_ = std::move(provider);
}

void ScriptVm::set_opcode_0045_condition_probe(Opcode0045ConditionProbe probe) {
    opcode_0045_condition_probe_ = std::move(probe);
}

void ScriptVm::set_opcode_00a5_player_state_code_two_probe(Opcode00A5PlayerStateCodeTwoProbe probe) {
    opcode_00a5_player_state_code_two_probe_ = std::move(probe);
}

void ScriptVm::set_clear_prints_adapter(ClearPrintsAdapter adapter) {
    clear_prints_adapter_ = std::move(adapter);
}

void ScriptVm::set_opcode_009f_character_dead_probe(Opcode009FCharacterDeadProbe probe) {
    opcode_009f_character_dead_probe_ = std::move(probe);
}

void ScriptVm::set_opcode_007f_button_pressed_probe(Opcode007FButtonPressedProbe probe) {
    opcode_007f_button_pressed_probe_ = std::move(probe);
}

void ScriptVm::set_fade_set_colour_adapter(FadeSetColourAdapter adapter) {
    fade_set_colour_adapter_ = std::move(adapter);
}

void ScriptVm::set_fade_start_adapter(FadeStartAdapter adapter) {
    fade_start_adapter_ = std::move(adapter);
}

void ScriptVm::set_fade_status_probe(FadeStatusProbe probe) {
    fade_status_probe_ = std::move(probe);
}

void ScriptVm::set_vehicle_delete_adapter(VehicleDeleteAdapter adapter) {
    vehicle_delete_adapter_ = std::move(adapter);
}

void ScriptVm::set_vehicle_release_mark_adapter(VehicleReleaseMarkAdapter adapter) {
    vehicle_release_mark_adapter_ = std::move(adapter);
}

void ScriptVm::set_global_byte_store_adapter(GlobalByteStoreAdapter adapter) {
    global_byte_store_adapter_ = std::move(adapter);
}

void ScriptVm::set_player_control_adapter(PlayerControlAdapter adapter) {
    player_control_adapter_ = std::move(adapter);
}

void ScriptVm::set_character_coordinate_adapter(CharacterCoordinateAdapter adapter) {
    character_coordinate_adapter_ = std::move(adapter);
}

void ScriptVm::set_pickup_model_object_resolver(PickupModelObjectResolver resolver) {
    pickup_model_object_resolver_ = std::move(resolver);
}

void ScriptVm::set_pickup_ground_height_probe(PickupGroundHeightProbe probe) {
    pickup_ground_height_probe_ = std::move(probe);
}

void ScriptVm::set_pickup_create_adapter(PickupCreateAdapter adapter) {
    pickup_create_adapter_ = std::move(adapter);
}

void ScriptVm::set_unique_jumps_total_adapter(UniqueJumpsTotalAdapter adapter) {
    unique_jumps_total_adapter_ = std::move(adapter);
}

void ScriptVm::set_display_text_adapter(DisplayTextAdapter adapter) {
    display_text_adapter_ = std::move(adapter);
}

void ScriptVm::set_clear_help_adapter(ClearHelpAdapter adapter) {
    clear_help_adapter_ = std::move(adapter);
}

void ScriptVm::set_pause_menu_toggle_adapter(PauseMenuToggleAdapter adapter) {
    pause_menu_toggle_adapter_ = std::move(adapter);
}

void ScriptVm::set_pad_state_adapter(PadStateAdapter adapter) {
    pad_state_adapter_ = std::move(adapter);
}

void ScriptVm::set_mission_launch_adapter(MissionLaunchAdapter adapter) {
    mission_launch_adapter_ = std::move(adapter);
}

void ScriptVm::set_script_termination_adapter(ScriptTerminationAdapter adapter) {
    script_termination_adapter_ = std::move(adapter);
}

void ScriptVm::register_handler(std::uint16_t opcode, OpcodeHandler handler) {
    if (opcode <= kLastPspOpcode)
        handlers_[opcode] = std::move(handler);
}

ProcessResult ScriptVm::process_one_command(ScriptThread& thread) {
    ProcessResult result{};
    std::uint16_t raw_opcode = 0;
    std::int16_t raw_signed = 0;
    if (!script_space_.read_i16(thread.ip, raw_signed)) {
        result.fault = VmFault::TruncatedBytecode;
        result.status = NativeHandlerStatus::Yield;
        return result;
    }
    raw_opcode = static_cast<std::uint16_t>(raw_signed);
    thread.not_flag = (raw_opcode & kOpcodeNotBit) != 0;
    const std::uint16_t opcode = raw_opcode & kOpcodeMask;
    result.last_opcode = opcode;

    if (opcode > kLastPspOpcode || !handlers_[opcode]) {
        result.fault = VmFault::UnsupportedOpcode;
        result.status = NativeHandlerStatus::Yield;
        return result;
    }

    const HandlerResult handler_result = handlers_[opcode](*this, thread, opcode);
    result.status = handler_result.status;
    result.fault = handler_result.fault;
    result.executed = 1;
    return result;
}

ProcessResult ScriptVm::process(ScriptThread& thread, std::size_t instruction_budget) {
    ProcessResult result{};
    if (!thread.is_active || thread.terminated) {
        result.status = NativeHandlerStatus::Yield;
        return result;
    }
    if (clock_() < thread.wake_time) {
        result.status = NativeHandlerStatus::Yield;
        return result;
    }
    for (std::size_t count = 0; count < instruction_budget; ++count) {
        result = process_one_command(thread);
        result.executed = count + 1;
        if (result.fault != VmFault::None || result.status != NativeHandlerStatus::Continue)
            return result;
    }
    result.status = NativeHandlerStatus::Yield;
    result.fault = VmFault::InstructionBudgetExhausted;
    return result;
}

bool ScriptVm::local_slot_is_valid(const ScriptThread& thread, std::uint32_t index) const {
    return index < thread.locals.size();
}

std::optional<ScriptVariableRef> ScriptVm::get_pointer_to_script_variable(
    ScriptThread& thread, std::uint32_t& ip, VmFault& fault) {
    std::uint8_t type = 0;
    if (!script_space_.read_u8(ip, type)) {
        fault = VmFault::TruncatedBytecode;
        return std::nullopt;
    }

    if (type < kArgumentGlobal) {
        if (type < kArgumentLocalArray) {
            const std::uint32_t local_index = type < kArgumentLocal
                ? kLowVariableTailBase + type
                : static_cast<std::uint32_t>(thread.locals_pointer) + (type - kArgumentLocal);
            if (!local_slot_is_valid(thread, local_index)) {
                fault = VmFault::VariableStorageOutOfRange;
                return std::nullopt;
            }
            return ScriptVariableRef{ScriptVariableRef::Storage::ThreadLocal, local_index};
        }

        std::uint8_t index_id = 0;
        std::uint8_t size = 0;
        if (!script_space_.read_u8(ip, index_id) || !script_space_.read_u8(ip, size)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        const std::uint32_t index_slot = static_cast<std::uint32_t>(thread.locals_pointer) + index_id;
        if (!local_slot_is_valid(thread, index_slot) || size == 0) {
            fault = VmFault::InvalidArrayIndex;
            return std::nullopt;
        }
        const std::int32_t array_index = thread.locals[index_slot];
        if (array_index < 0 || static_cast<std::uint32_t>(array_index) >= size) {
            fault = VmFault::InvalidArrayIndex;
            return std::nullopt;
        }
        const std::uint32_t local_index = static_cast<std::uint32_t>(thread.locals_pointer) +
            (type - kArgumentLocalArray) + static_cast<std::uint32_t>(array_index);
        if (!local_slot_is_valid(thread, local_index)) {
            fault = VmFault::VariableStorageOutOfRange;
            return std::nullopt;
        }
        return ScriptVariableRef{ScriptVariableRef::Storage::ThreadLocal, local_index};
    }

    std::uint8_t index_in_block = 0;
    if (!script_space_.read_u8(ip, index_in_block)) {
        fault = VmFault::TruncatedBytecode;
        return std::nullopt;
    }

    if (type < kArgumentGlobalArray) {
        const std::uint32_t address = 4U *
            (static_cast<std::uint32_t>(type - kArgumentGlobal) * 0x100U + index_in_block);
        return ScriptVariableRef{ScriptVariableRef::Storage::ScriptSpace, address};
    }

    std::uint8_t index_id = 0;
    std::uint8_t size = 0;
    if (!script_space_.read_u8(ip, index_id) || !script_space_.read_u8(ip, size)) {
        fault = VmFault::TruncatedBytecode;
        return std::nullopt;
    }
    const std::uint32_t index_slot = static_cast<std::uint32_t>(thread.locals_pointer) + index_id;
    if (!local_slot_is_valid(thread, index_slot) || size == 0) {
        fault = VmFault::InvalidArrayIndex;
        return std::nullopt;
    }
    const std::int32_t array_index = thread.locals[index_slot];
    if (array_index < 0 || static_cast<std::uint32_t>(array_index) >= size) {
        fault = VmFault::InvalidArrayIndex;
        return std::nullopt;
    }
    const std::uint32_t address = 4U *
        (static_cast<std::uint32_t>(type - kArgumentGlobalArray) * 0x100U +
         static_cast<std::uint32_t>(array_index) + index_in_block);
    return ScriptVariableRef{ScriptVariableRef::Storage::ScriptSpace, address};
}

bool ScriptVm::read_script_variable(const ScriptThread& thread, const ScriptVariableRef& variable,
                                    std::int32_t& value, VmFault& fault) const {
    if (variable.storage == ScriptVariableRef::Storage::ThreadLocal) {
        if (!local_slot_is_valid(thread, variable.index)) {
            fault = VmFault::VariableStorageOutOfRange;
            return false;
        }
        value = thread.locals[variable.index];
        return true;
    }
    if (!script_space_.read_i32_at(variable.index, value)) {
        fault = VmFault::VariableStorageOutOfRange;
        return false;
    }
    return true;
}

bool ScriptVm::write_script_variable(ScriptThread& thread, const ScriptVariableRef& variable,
                                     std::int32_t value, VmFault& fault) {
    if (variable.storage == ScriptVariableRef::Storage::ThreadLocal) {
        if (!local_slot_is_valid(thread, variable.index)) {
            fault = VmFault::VariableStorageOutOfRange;
            return false;
        }
        thread.locals[variable.index] = value;
        return true;
    }
    if (!script_space_.write_i32_at(variable.index, value)) {
        fault = VmFault::VariableStorageOutOfRange;
        return false;
    }
    return true;
}

std::optional<std::int32_t> ScriptVm::read_parameter_at(
    ScriptThread& thread, std::uint32_t& ip, VmFault& fault) {
    std::uint8_t tag = 0;
    if (!script_space_.read_u8(ip, tag)) {
        fault = VmFault::TruncatedBytecode;
        return std::nullopt;
    }
    switch (tag) {
    case kArgumentIntZero:
    case kArgumentFloatZero:
        return 0;
    case kArgumentFloat1Byte: {
        std::uint8_t value = 0;
        if (!script_space_.read_u8(ip, value)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        return static_cast<std::int32_t>(static_cast<std::uint32_t>(value) << 24U);
    }
    case kArgumentFloat2Bytes: {
        std::int16_t value = 0;
        if (!script_space_.read_i16(ip, value)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        return static_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::uint16_t>(value)) << 16U);
    }
    case kArgumentFloat3Bytes: {
        std::uint8_t low = 0;
        std::int16_t high = 0;
        if (!script_space_.read_u8(ip, low) || !script_space_.read_i16(ip, high)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        const std::uint32_t raw = (static_cast<std::uint32_t>(low) << 8U) |
            (static_cast<std::uint32_t>(static_cast<std::uint16_t>(high)) << 16U);
        return static_cast<std::int32_t>(raw);
    }
    case kArgumentInt32: {
        std::int32_t value = 0;
        if (!script_space_.read_i32(ip, value)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        return value;
    }
    case kArgumentInt8: {
        std::int8_t value = 0;
        if (!script_space_.read_i8(ip, value)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        return value;
    }
    case kArgumentInt16: {
        std::int16_t value = 0;
        if (!script_space_.read_i16(ip, value)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        return value;
    }
    case kArgumentFloat: {
        std::int32_t value = 0;
        if (!script_space_.read_i32(ip, value)) {
            fault = VmFault::TruncatedBytecode;
            return std::nullopt;
        }
        return value;
    }
    case kArgumentString: {
        // PSP CollectParameters 0x0006189C skips the tag, saves this
        // ScriptSpace-relative address, and advances through the terminating
        // NUL. The portable raw-int32 ABI represents that address directly.
        const std::uint32_t string_offset = ip;
        std::uint8_t character = 0;
        do {
            if (!script_space_.read_u8(ip, character)) {
                fault = VmFault::TruncatedBytecode;
                return std::nullopt;
            }
        } while (character != 0);
        if (string_offset > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            fault = VmFault::VariableStorageOutOfRange;
            return std::nullopt;
        }
        return static_cast<std::int32_t>(string_offset);
    }
    default:
        // FUN_0005E71C rewinds its private IP by one byte and delegates all
        // non-immediate encodings to FUN_0005DA7C.
        --ip;
        const std::optional<ScriptVariableRef> variable = get_pointer_to_script_variable(thread, ip, fault);
        if (!variable)
            return std::nullopt;
        std::int32_t value = 0;
        if (!read_script_variable(thread, *variable, value, fault))
            return std::nullopt;
        return value;
    }
}

std::optional<std::int32_t> ScriptVm::collect_next_parameter_without_increasing_pc(
    ScriptThread& thread, std::uint32_t parameter_ip, VmFault& fault) {
    return read_parameter_at(thread, parameter_ip, fault);
}

std::optional<std::int32_t> ScriptVm::read_parameter(ScriptThread& thread, VmFault& fault) {
    std::uint8_t tag = 0;
    if (!script_space_.read_u8(thread.ip, tag)) {
        fault = VmFault::TruncatedBytecode;
        return std::nullopt;
    }
    if (tag == kArgumentVcsPluralZero)
        return 0;
    --thread.ip;
    return read_parameter_at(thread, thread.ip, fault);
}

bool ScriptVm::collect_parameters(ScriptThread& thread, std::int16_t total,
                                  std::vector<std::int32_t>& parameters, VmFault& fault) {
    parameters.clear();
    if (total < 0) {
        fault = VmFault::InvalidVariableReference;
        return false;
    }
    parameters.reserve(static_cast<std::size_t>(total));
    for (std::int16_t index = 0; index < total; ++index) {
        std::uint8_t next_tag = 0;
        if (!script_space_.read_u8(thread.ip, next_tag)) {
            fault = VmFault::TruncatedBytecode;
            return false;
        }
        if (next_tag == kArgumentVcsPluralZero) {
            parameters.push_back(0);
            continue;
        }
        --thread.ip;
        const std::optional<std::int32_t> value = read_parameter(thread, fault);
        if (!value)
            return false;
        parameters.push_back(*value);
    }
    return true;
}

bool ScriptVm::store_parameters(ScriptThread& thread, const std::vector<std::int32_t>& parameters,
                                VmFault& fault) {
    for (const std::int32_t value : parameters) {
        const std::optional<ScriptVariableRef> variable =
            get_pointer_to_script_variable(thread, thread.ip, fault);
        if (!variable || !write_script_variable(thread, *variable, value, fault))
            return false;
    }
    return true;
}

std::optional<std::int32_t> ScriptVm::read_integer_parameter(ScriptThread& thread, VmFault& fault) {
    return read_parameter(thread, fault);
}

bool ScriptVm::set_instruction_pointer(ScriptThread& thread, std::int32_t encoded_target, VmFault& fault) {
    const std::int64_t target = encoded_target >= 0
        ? static_cast<std::int64_t>(encoded_target)
        : static_cast<std::int64_t>(script_space_.main_script_size()) - static_cast<std::int64_t>(encoded_target);
    if (target < 0 || target > std::numeric_limits<std::uint32_t>::max() || !script_space_.is_valid_ip(static_cast<std::uint32_t>(target))) {
        fault = VmFault::InvalidScriptTarget;
        return false;
    }
    thread.ip = static_cast<std::uint32_t>(target);
    return true;
}

HandlerResult ScriptVm::handle_nop(ScriptThread&) {
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_wait(ScriptThread& thread) {
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> delay = read_integer_parameter(thread, fault);
    if (!delay)
        return HandlerResult::Fault(fault);
    thread.wake_time = clock_() + static_cast<std::uint32_t>(*delay);
    thread.wait_flag = false;
    return HandlerResult::Yield();
}

HandlerResult ScriptVm::handle_goto(ScriptThread& thread) {
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> target = read_integer_parameter(thread, fault);
    if (!target)
        return HandlerResult::Fault(fault);
    if (!set_instruction_pointer(thread, *target, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_assign(ScriptThread& thread) {
    // PSP 002B7448/002B74A8 and PS2 003B14B0/003B2A20 resolve a destination
    // first, collect one raw parameter, then write that dword unchanged. The
    // table aliases differ in SCM type notation, not in this VM action.
    VmFault fault = VmFault::None;
    const std::optional<ScriptVariableRef> destination =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!destination)
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> value = read_parameter(thread, fault);
    if (!value || !write_script_variable(thread, *destination, *value, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_integer_arithmetic(ScriptThread& thread, std::uint16_t opcode) {
    // PSP forced-linear windows 002B75D8/76A8/777C and PS2 003B1610/1700/17F0
    // expose subu, mult/mflo and div/mflo. Addition is the same raw dword
    // operation already proved at PSP 002B7508 / PS2 003B1520.
    VmFault fault = VmFault::None;
    const std::optional<ScriptVariableRef> destination =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!destination)
        return HandlerResult::Fault(fault);
    std::int32_t current = 0;
    if (!read_script_variable(thread, *destination, current, fault))
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> value = read_parameter(thread, fault);
    if (!value)
        return HandlerResult::Fault(fault);
    std::int32_t result = 0;
    switch (opcode) {
    case kOpcodeAddValueToIntVar:
    case kOpcodeAddIntVarToIntVar:
        result = static_cast<std::int32_t>(static_cast<std::uint32_t>(current) + static_cast<std::uint32_t>(*value));
        break;
    case kOpcodeSubValueFromIntVar:
    case kOpcodeIntegerSubtractDispatchAlias:
        result = static_cast<std::int32_t>(static_cast<std::uint32_t>(current) - static_cast<std::uint32_t>(*value));
        break;
    case kOpcodeMultiplyIntVarByValue:
    case kOpcodeIntegerMultiplyDispatchAlias:
        result = static_cast<std::int32_t>(static_cast<std::uint64_t>(static_cast<std::uint32_t>(current)) *
                                           static_cast<std::uint64_t>(static_cast<std::uint32_t>(*value)));
        break;
    case kOpcodeDivideIntVarByValue:
    case kOpcodeIntegerDivideDispatchAlias:
        // Both binaries issue native `div` without a language-level guard.
        // The portable model fails loudly rather than inventing hardware
        // quotient semantics for zero and INT_MIN / -1.
        if (*value == 0)
            return HandlerResult::Fault(VmFault::IntegerDivisionByZero);
        if (current == std::numeric_limits<std::int32_t>::min() && *value == -1)
            return HandlerResult::Fault(VmFault::IntegerDivisionOverflow);
        result = current / *value; // C++17 truncates toward zero, like MIPS div.
        break;
    default:
        return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }
    if (!write_script_variable(thread, *destination, result, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_float_arithmetic(ScriptThread& thread, std::uint16_t opcode) {
    // PSP forced-linear windows 002B7570/7640/7714/77E8 and PS2
    // 003B1598/1688/1778/1878 use scalar single-precision FPU arithmetic.
    VmFault fault = VmFault::None;
    const std::optional<ScriptVariableRef> destination =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!destination)
        return HandlerResult::Fault(fault);
    std::int32_t current_raw = 0;
    if (!read_script_variable(thread, *destination, current_raw, fault))
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> value_raw = read_parameter(thread, fault);
    if (!value_raw)
        return HandlerResult::Fault(fault);

    const float current = float_from_raw_parameter(current_raw);
    const float value = float_from_raw_parameter(*value_raw);
    float result = 0.0F;
    switch (opcode) {
    case kOpcodeAddValueToFloatVar:
    case kOpcodeFloatAddDispatchAlias: result = current + value; break;
    case kOpcodeSubValueFromFloatVar:
    case kOpcodeFloatSubtractDispatchAlias: result = current - value; break;
    case kOpcodeMultiplyFloatVarByValue:
    case kOpcodeFloatMultiplyDispatchAlias: result = current * value; break;
    case kOpcodeDivideFloatVarByValue:
    case kOpcodeFloatDivideDispatchAlias: result = current / value; break;
    default: return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }
    if (!write_script_variable(thread, *destination, raw_parameter_from_float(result), fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_timed_float_arithmetic(ScriptThread& thread, std::uint16_t opcode) {
    // PSP 002B7EA8/002B7F18 and PS2 003B1D20/003B1DA8 resolve one
    // destination, collect one binary32 value, then multiply that value by
    // the engine frame-time scalar before adding/subtracting it. The scalar is
    // not VM-owned (PSP gp+0x1E1C; PS2 DAT_004CD410), so it is a host adapter.
    VmFault fault = VmFault::None;
    const std::optional<ScriptVariableRef> destination =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!destination)
        return HandlerResult::Fault(fault);
    std::int32_t current_raw = 0;
    if (!read_script_variable(thread, *destination, current_raw, fault))
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> value_raw = read_parameter(thread, fault);
    if (!value_raw)
        return HandlerResult::Fault(fault);
    if (!time_step_provider_)
        return HandlerResult::Fault(VmFault::TimedArithmeticRequiresTimeStep);

    const float current = float_from_raw_parameter(current_raw);
    const float timed_value = float_from_raw_parameter(*value_raw) * time_step_provider_();
    float result = 0.0F;
    switch (opcode) {
    case kOpcodeAddTimedValueToFloatVar:
    case kOpcodeAddTimedFloatVarToFloatVar:
        result = current + timed_value;
        break;
    case kOpcodeSubTimedValueFromFloatVar:
    case kOpcodeSubTimedFloatVarFromFloatVar:
        result = current - timed_value;
        break;
    default:
        return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }
    if (!write_script_variable(thread, *destination, raw_parameter_from_float(result), fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_numeric_conversion(ScriptThread& thread, std::uint16_t opcode) {
    // PSP 002B7F88/002B7FE8 and PS2 003B1E30/003B1E98 each resolve two
    // variable pointers through the small forwarding wrapper around the
    // confirmed GetPointerToScriptVariable primitive, then return 0.
    VmFault fault = VmFault::None;
    const std::optional<ScriptVariableRef> destination =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!destination)
        return HandlerResult::Fault(fault);
    const std::optional<ScriptVariableRef> source =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!source)
        return HandlerResult::Fault(fault);
    std::int32_t source_raw = 0;
    if (!read_script_variable(thread, *source, source_raw, fault))
        return HandlerResult::Fault(fault);

    std::int32_t result = 0;
    switch (opcode) {
    case kOpcodeCSetVarIntToVarFloat: {
        // MIPS trunc.w.s (not floor): normal values round toward zero.
        const double truncated = std::trunc(static_cast<double>(float_from_raw_parameter(source_raw)));
        if (!std::isfinite(truncated) ||
            truncated < static_cast<double>(std::numeric_limits<std::int32_t>::min()) ||
            truncated > static_cast<double>(std::numeric_limits<std::int32_t>::max()))
            return HandlerResult::Fault(VmFault::FloatToIntegerConversionOutOfRange);
        result = static_cast<std::int32_t>(truncated);
        break;
    }
    case kOpcodeCSetVarFloatToVarInt:
        result = raw_parameter_from_float(static_cast<float>(source_raw));
        break;
    default:
        return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }
    if (!write_script_variable(thread, *destination, result, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_absolute_value(ScriptThread& thread, std::uint16_t opcode) {
    // PSP 002B8048/002B8088 and PS2 003B1F00/003B1F38 resolve one variable,
    // conditionally negate it and return 0. Integer ABS uses subu, so INT_MIN
    // is preserved as its modulo-2^32 result; float ABS preserves -0 and NaN.
    VmFault fault = VmFault::None;
    const std::optional<ScriptVariableRef> variable =
        get_pointer_to_script_variable(thread, thread.ip, fault);
    if (!variable)
        return HandlerResult::Fault(fault);
    std::int32_t value_raw = 0;
    if (!read_script_variable(thread, *variable, value_raw, fault))
        return HandlerResult::Fault(fault);

    std::int32_t result = 0;
    switch (opcode) {
    case kOpcodeAbsInt:
        result = value_raw < 0
            ? static_cast<std::int32_t>(0U - static_cast<std::uint32_t>(value_raw))
            : value_raw;
        break;
    case kOpcodeAbsFloat: {
        float value = float_from_raw_parameter(value_raw);
        if (value < 0.0F)
            value = -value;
        result = raw_parameter_from_float(value);
        break;
    }
    default:
        return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }
    if (!write_script_variable(thread, *variable, result, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_integral_or_raw_equality_condition(ScriptThread& thread, std::uint16_t opcode) {
    // PSP forced-linear windows 002B7880 (000F/0010), 002B7A20 (0015) and
    // 002B7BC4 (001B/02DB), corroborated directly by PS2 handlers
    // 003B18F8/003B19C8/003B1C50, all call CollectParameters with count 2,
    // form a signed predicate, then enter the exact shared condition reducer.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, 2, parameters, fault) || parameters.size() != 2)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);

    bool base_condition = false;
    switch (opcode) {
    case kOpcodeIsIntVarGreaterThanNumber:
    case kOpcodeIsNumberGreaterThanIntVar:
    case kOpcodeIsIntVarGreaterThanIntVar:
        // The handler is shared because source-kind notation does not survive
        // CollectParameters: both commands evaluate parameter[0] > parameter[1].
        base_condition = parameters[0] > parameters[1];
        break;
    case kOpcodeIsIntVarGreaterOrEqualToNumber:
    case kOpcodeIsNumberGreaterOrEqualToIntVar:
    case kOpcodeIsIntVarGreaterOrEqualToIntVar:
        base_condition = parameters[0] >= parameters[1];
        break;
    case kOpcodeIsIntVarEqualToNumber:
    case kOpcodeIsIntVarEqualToIntVar:
    case kOpcodeIsFloatVarEqualToNumber:
    case kOpcodeIsFloatVarEqualToFloatVar:
    case kOpcodeIsIntVarEqualToConstant:
        base_condition = parameters[0] == parameters[1];
        break;
    default:
        return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }

    reduce_condition_result(thread, base_condition);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_float_condition(ScriptThread& thread, std::uint16_t opcode) {
    // PSP 002B7948 executes c.le.s then an inverted branch-likely for the
    // `>` family; 002B7AEC does the same with c.lt.s for the `>=` family.
    // `!(left <= right)` / `!(left < right)` intentionally preserve the
    // native unordered (NaN) branch result, unlike a naïve `left > right`.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, 2, parameters, fault) || parameters.size() != 2)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);

    const float left = float_from_raw_parameter(parameters[0]);
    const float right = float_from_raw_parameter(parameters[1]);
    bool base_condition = false;
    switch (opcode) {
    case kOpcodeIsFloatVarGreaterThanNumber:
    case kOpcodeIsNumberGreaterThanFloatVar:
    case kOpcodeIsFloatVarGreaterThanFloatVar:
        base_condition = !(left <= right);
        break;
    case kOpcodeIsFloatVarGreaterOrEqualToNumber:
    case kOpcodeIsNumberGreaterOrEqualToFloatVar:
    case kOpcodeIsFloatVarGreaterOrEqualToFloatVar:
        base_condition = !(left < right);
        break;
    default:
        return HandlerResult::Fault(VmFault::UnsupportedOpcode);
    }

    reduce_condition_result(thread, base_condition);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_0045_is_char_still_alive(ScriptThread& thread) {
    // PSP 003233F0 and PS2 00412FC8 both collect one character handle, resolve
    // it through engine-owned pools/state helpers, apply raw NOT, then enter
    // the exact shared condition reducer. Only that engine predicate is host
    // supplied here; no binary Ped/Pool layout leaks into the portable VM.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> char_handle = read_parameter(thread, fault);
    if (!char_handle)
        return HandlerResult::Fault(fault);
    if (!opcode_0045_condition_probe_)
        return HandlerResult::Fault(VmFault::Opcode0045RequiresConditionProbe);
    reduce_condition_result(thread, opcode_0045_condition_probe_(*this, thread, *char_handle));
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_00a5_player_state_code_two_condition(ScriptThread& thread) {
    // PSP 001DA7C4 / PS2 002D2F60 collect one player selector, resolve the
    // player and compare its recovered native state byte with exactly 2. The
    // raw NOT bit and common condition reducer are VM-owned.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> player_selector = read_parameter(thread, fault);
    if (!player_selector)
        return HandlerResult::Fault(fault);
    if (!opcode_00a5_player_state_code_two_probe_)
        return HandlerResult::Fault(VmFault::Opcode00A5RequiresPlayerStateCodeTwoProbe);
    reduce_condition_result(thread, opcode_00a5_player_state_code_two_probe_(*this, thread, *player_selector));
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_clear_prints(ScriptThread&) {
    // PSP 002EB77C / PS2 003DAE38 consume no script bytes after the opcode,
    // invoke one native UI clear path and return 0. Its owner is host work.
    if (!clear_prints_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0059RequiresClearPrintsAdapter);
    clear_prints_adapter_();
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_009f_character_dead_condition(ScriptThread& thread) {
    // PSP 00323E98 / PS2 00413968 collect one character handle, evaluate a
    // multi-branch engine dead-state predicate, then use the shared condition
    // ABI. The engine object layout and status meanings stay host-owned.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> character_handle = read_parameter(thread, fault);
    if (!character_handle)
        return HandlerResult::Fault(fault);
    if (!opcode_009f_character_dead_probe_)
        return HandlerResult::Fault(VmFault::Opcode009FRequiresCharacterDeadProbe);
    reduce_condition_result(thread, opcode_009f_character_dead_probe_(*this, thread, *character_handle));
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_007f_is_button_pressed(ScriptThread& thread) {
    // PSP forced-linear window 002ACC1C and PS2 003A7C70 both collect two
    // parameters, truncate ScriptParams[0]/[1] to uint16, invoke an input
    // helper, then apply raw NOT and the exact shared condition reducer.
    // Native argument meanings are outside the VM boundary.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, 2, parameters, fault) || parameters.size() != 2)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);
    if (!opcode_007f_button_pressed_probe_)
        return HandlerResult::Fault(VmFault::Opcode007FRequiresButtonPressedProbe);
    reduce_condition_result(
        thread,
        opcode_007f_button_pressed_probe_(
            *this,
            thread,
            static_cast<std::uint16_t>(parameters[0]),
            static_cast<std::uint16_t>(parameters[1])));
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_set_fading_colour(ScriptThread& thread) {
    // PSP 0018BC64 / PS2 00287BE0 collect exactly three raw values, mask each
    // to its low byte and call the engine fade object's colour method.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, 3, parameters, fault) || parameters.size() != 3)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);
    if (!fade_set_colour_adapter_)
        return HandlerResult::Fault(VmFault::Opcode00C8RequiresFadeColourAdapter);
    fade_set_colour_adapter_(
        static_cast<std::uint8_t>(parameters[0]),
        static_cast<std::uint8_t>(parameters[1]),
        static_cast<std::uint8_t>(parameters[2]));
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_do_fade(ScriptThread& thread) {
    // PSP 0018BCC8 / PS2 00287C38 collect two raw values. The first is
    // converted with the exact native threshold: <=2 becomes 0.0f, otherwise
    // single-precision value * 0.001f. The second is sign-extended to int16.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, 2, parameters, fault) || parameters.size() != 2)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);
    if (!fade_start_adapter_)
        return HandlerResult::Fault(VmFault::Opcode00C9RequiresFadeStartAdapter);
    const float duration_seconds = parameters[0] <= 2
        ? 0.0F
        : static_cast<float>(parameters[0]) * 0.001F;
    const std::int16_t mode = static_cast<std::int16_t>(static_cast<std::uint16_t>(parameters[1]));
    fade_start_adapter_(duration_seconds, mode);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_get_fading_status(ScriptThread& thread) {
    // PSP 0018BD58 / PS2 00287CC8 query the fixed fade object, then apply raw
    // NOT and the exact shared condition reducer without SCM parameters.
    if (!fade_status_probe_)
        return HandlerResult::Fault(VmFault::Opcode00CARequiresFadeStatusProbe);
    reduce_condition_result(thread, fade_status_probe_());
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_delete_vehicle(ScriptThread& thread) {
    // PSP 002E03C8 / PS2 003D0B60 collect exactly one value into the shared
    // parameter buffer, resolve it through the vehicle pool, conditionally
    // perform the observed +0x20A cleanup path and remove a non-null entity.
    // The host owns all pool, cleanup-list and entity-lifetime details.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> vehicle_handle = read_parameter(thread, fault);
    if (!vehicle_handle)
        return HandlerResult::Fault(fault);
    if (!vehicle_delete_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0049RequiresVehicleDeleteAdapter);
    vehicle_delete_adapter_(*vehicle_handle, thread.mission_launch_requested);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_vehicle_release_mark(ScriptThread& thread) {
    // PSP 002E1700 / PS2 003D2218 collect exactly one raw handle, resolve it
    // through the vehicle pool and invoke a release-mark helper.  The native
    // +0x20A branch is separate context cleanup, not entity destruction.
    // Pool ownership, release policy and cleanup-list semantics stay host-side.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> vehicle_handle = read_parameter(thread, fault);
    if (!vehicle_handle)
        return HandlerResult::Fault(fault);
    if (!vehicle_release_mark_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0113RequiresVehicleReleaseMarkAdapter);
    vehicle_release_mark_adapter_(*vehicle_handle, thread.mission_launch_requested);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_global_byte_store(ScriptThread& thread) {
    // PSP 0008273C / PS2 00185408 collect exactly two values. Parameter 0 is
    // an offset into an engine-owned byte region; parameter 1 is narrowed to
    // its low byte for the store. The global region remains host-owned.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> slot_offset = read_parameter(thread, fault);
    if (!slot_offset)
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> raw_value = read_parameter(thread, fault);
    if (!raw_value)
        return HandlerResult::Fault(fault);
    if (!global_byte_store_adapter_)
        return HandlerResult::Fault(VmFault::Opcode04E5RequiresGlobalByteStoreAdapter);
    global_byte_store_adapter_(*slot_offset, static_cast<std::uint8_t>(*raw_value));
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_player_control(ScriptThread& thread) {
    // PSP 001DA8A4 / PS2 002D30E8 collect two values, resolve parameter 0
    // through the player table and branch on parameter 1 being zero. The
    // safety/camera/reset branches stay in the host player-control adapter.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> player_selector = read_parameter(thread, fault);
    if (!player_selector)
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> enabled_raw = read_parameter(thread, fault);
    if (!enabled_raw)
        return HandlerResult::Fault(fault);
    if (!player_control_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0107RequiresPlayerControlAdapter);
    player_control_adapter_(*player_selector, *enabled_raw != 0);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_character_coordinate_placement(ScriptThread& thread) {
    // PSP 003232F8 / PS2 00412DE8 collect handle + x/y/z binary32 payloads.
    // They resolve ground Z only for the native -100.0 / <= -250.0 sentinel
    // region, then enter an engine-owned character placement path.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> character_handle = read_parameter(thread, fault);
    if (!character_handle)
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> x_raw = read_parameter(thread, fault);
    if (!x_raw)
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> y_raw = read_parameter(thread, fault);
    if (!y_raw)
        return HandlerResult::Fault(fault);
    const std::optional<std::int32_t> z_raw = read_parameter(thread, fault);
    if (!z_raw)
        return HandlerResult::Fault(fault);
    if (!character_coordinate_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0044RequiresCharacterCoordinateAdapter);
    const float z = float_from_raw_parameter(*z_raw);
    const bool resolve_ground_height = z == -100.0f || z <= -250.0f;
    character_coordinate_adapter_(*character_handle, *x_raw, *y_raw, *z_raw, resolve_ground_height);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_create_pickup(ScriptThread& thread, bool has_ammo_parameter) {
    // 013F PSP 001DDCD0 / PS2 002D7CE0 collects five values, while 01F9 PSP
    // 001DE044 / PS2 002D8050 collects six and sends parameter[2] as ammo to
    // the very same native pickup constructor. Both share negative-model,
    // ground-Z and one StoreParameters output behavior.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, has_ammo_parameter ? 6 : 5, parameters, fault))
        return HandlerResult::Fault(fault);

    std::int32_t model_id = parameters[0];
    if (model_id < 0) {
        if (!pickup_model_object_resolver_)
            return HandlerResult::Fault(VmFault::Opcode013FRequiresPickupModelObjectResolver);
        const std::optional<std::int32_t> resolved = pickup_model_object_resolver_(-model_id);
        if (!resolved)
            return HandlerResult::Fault(VmFault::Opcode013FRequiresPickupModelObjectResolver);
        model_id = *resolved;
    }

    const std::size_t coordinate_index = has_ammo_parameter ? 3 : 2;
    const std::int32_t ammo = has_ammo_parameter ? parameters[2] : 0;
    const float x = float_from_raw_parameter(parameters[coordinate_index]);
    const float y = float_from_raw_parameter(parameters[coordinate_index + 1]);
    float z = float_from_raw_parameter(parameters[coordinate_index + 2]);
    if (z == -100.0F || z <= -250.0F) {
        if (!pickup_ground_height_probe_)
            return HandlerResult::Fault(VmFault::Opcode013FRequiresPickupGroundHeightProbe);
        const std::optional<float> ground_height = pickup_ground_height_probe_(x, y);
        if (!ground_height)
            return HandlerResult::Fault(VmFault::Opcode013FRequiresPickupGroundHeightProbe);
        z = *ground_height + 0.5F;
    }
    if (!pickup_create_adapter_)
        return HandlerResult::Fault(VmFault::Opcode013FRequiresPickupCreateAdapter);

    const std::int32_t pickup_handle = pickup_create_adapter_(
        model_id, static_cast<std::uint8_t>(parameters[1]), ammo, x, y, z);
    if (!store_parameters(thread, {pickup_handle}, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_set_unique_jumps_total(ScriptThread& thread) {
    // PSP 00081FBC / PS2 00184C40 collect exactly one raw dword, write it to
    // the same stat-global role and return native CONTINUE. Exact reLCS source
    // identifies this aggregate as TotalNumberOfUniqueJumps.
    std::vector<std::int32_t> parameters;
    VmFault fault = VmFault::None;
    if (!collect_parameters(thread, 1, parameters, fault))
        return HandlerResult::Fault(fault);
    if (!unique_jumps_total_adapter_)
        return HandlerResult::Fault(VmFault::Opcode01E7RequiresUniqueJumpsTotalAdapter);
    unique_jumps_total_adapter_(parameters[0]);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_display_text(ScriptThread& thread) {
    // PSP 002EC548 / PS2 003DBB10 collect the two coordinate payloads first,
    // then perform a separate one-parameter collection for the text key. The
    // native handler resolves/copies UTF-16 text and queues a bounded display
    // record; that text-manager state is intentionally host-owned here.
    VmFault fault = VmFault::None;
    std::vector<std::int32_t> coordinates;
    if (!collect_parameters(thread, 2, coordinates, fault) || coordinates.size() != 2)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);
    const std::optional<std::int32_t> text_key = read_parameter(thread, fault);
    if (!text_key)
        return HandlerResult::Fault(fault);
    if (!display_text_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0203RequiresDisplayTextAdapter);
    display_text_adapter_(coordinates[0], coordinates[1], *text_key);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_clear_help(ScriptThread&) {
    // PSP 002ED0E8 / PS2 003DC6D0 ensure their help/HUD manager exists, invoke
    // two zero-argument clear paths, then return 0. Both manager lifecycle and
    // the two native channels are host integration work.
    if (!clear_help_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0268RequiresClearHelpAdapter);
    clear_help_adapter_();
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_pause_menu_toggle(ScriptThread& thread) {
    // PSP 0031A0C0 / PS2 00406240 collect one raw dword, normalize it with a
    // zero/non-zero branch, and pass exactly false/true to the native pause
    // menu object's setter. Menu construction/state ownership remain host work.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> parameter = read_parameter(thread, fault);
    if (!parameter)
        return HandlerResult::Fault(fault);
    if (!pause_menu_toggle_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0401RequiresPauseMenuToggleAdapter);
    pause_menu_toggle_adapter_(*parameter != 0);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_get_game_timer(ScriptThread& thread) {
    // PSP 002F0118 / PS2 003DF8C8 copy the same engine timer scalar used by
    // WAIT's clock abstraction into ScriptParams, then StoreParameters(1).
    // Preserve it as a raw dword, including its native signed interpretation.
    const std::uint32_t timer_bits = clock_();
    std::int32_t timer_value = 0;
    std::memcpy(&timer_value, &timer_bits, sizeof(timer_value));
    VmFault fault = VmFault::None;
    if (!store_parameters(thread, {timer_value}, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_get_pad_state(ScriptThread& thread) {
    // PSP 002ACDEC / PS2 003A7E38 collect exactly one selector, use an unsigned
    // jump table only for 0..44 to overwrite ScriptParams[0] with a pad result,
    // then StoreParameters(1). Out-of-range selectors skip the jump table and
    // are consequently stored unchanged by the native code.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> selector_raw = read_parameter(thread, fault);
    if (!selector_raw)
        return HandlerResult::Fault(fault);

    std::int32_t result = *selector_raw;
    const std::uint32_t selector = static_cast<std::uint32_t>(*selector_raw);
    if (selector < 0x2DU) {
        if (!pad_state_adapter_)
            return HandlerResult::Fault(VmFault::Opcode0377RequiresPadStateAdapter);
        result = pad_state_adapter_(selector);
    }
    if (!store_parameters(thread, {result}, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_launch_mission(ScriptThread& thread) {
    // PSP 002B8250 / PS2 003B2320 collect one mission identifier, resolve a
    // target running script through an engine-owned helper, set that target's
    // +0x20A byte to one and return 0. The resolver/lifetime stay with host.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> mission_id = read_parameter(thread, fault);
    if (!mission_id)
        return HandlerResult::Fault(fault);
    if (!mission_launch_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0079RequiresMissionLaunchAdapter);
    ScriptThread* const target = mission_launch_adapter_(*this, *mission_id);
    if (!target)
        return HandlerResult::Fault(VmFault::MissionLaunchAdapterReturnedNull);
    target->mission_launch_requested = true;
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_script_name(ScriptThread& thread) {
    // PSP 002B8360 and PS2 003B2440 collect one pointer-like raw parameter,
    // invoke the local strncpy implementation for exactly eight bytes into
    // thread+0x20F, then lowercase ASCII A..Z in all eight field positions.
    // The direct string tag is a ScriptSpace-relative offset in this portable
    // model; source zero is a native null-pointer path and faults explicitly.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> source_raw = read_parameter(thread, fault);
    if (!source_raw)
        return HandlerResult::Fault(fault);
    if (*source_raw <= 0 || !script_space_.is_valid_ip(static_cast<std::uint32_t>(*source_raw)))
        return HandlerResult::Fault(VmFault::InvalidScriptStringAddress);

    thread.script_name.fill('\0');
    std::uint32_t source_ip = static_cast<std::uint32_t>(*source_raw);
    for (std::size_t index = 0; index < thread.script_name.size(); ++index) {
        std::uint8_t character = 0;
        if (!script_space_.read_u8(source_ip, character))
            return HandlerResult::Fault(VmFault::InvalidScriptStringAddress);
        thread.script_name[index] = static_cast<char>(character);
        if (character == 0)
            break;
    }
    for (char& character : thread.script_name) {
        const unsigned char raw = static_cast<unsigned char>(character);
        if (raw >= static_cast<unsigned char>('A') && raw <= static_cast<unsigned char>('Z'))
            character = static_cast<char>(raw + static_cast<unsigned char>('a' - 'A'));
    }
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_mission_streaming_boundary(ScriptThread& thread) {
    // PSP 002B8400 / PS2 003B24E0 first collect exactly one raw selector,
    // then leave the VM core: table range selection, script-buffer lifetime,
    // target construction, request flags, global mission state and re-entrant
    // Process(target) are one inseparable host subsystem. Consume the proved
    // input so diagnostics retain an exact bytecode boundary, then fail loudly
    // rather than disguising this pipeline as a narrow launch callback.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> selector = read_parameter(thread, fault);
    if (!selector)
        return HandlerResult::Fault(fault);
    return HandlerResult::Fault(VmFault::Opcode0289RequiresMissionStreamingSubsystem);
}

HandlerResult ScriptVm::handle_is_this_script_a_mission(ScriptThread& thread) {
    // PSP 002B89FC and PS2 003B2B20 load byte +0x217 from the globally active
    // running script, then execute the exact raw-NOT/+0x206/+0x209 condition
    // reducer. process(thread) is the portable active-thread boundary.
    reduce_condition_result(thread, thread.is_mission_script);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_goto_if_false(ScriptThread& thread) {
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> target = read_integer_parameter(thread, fault);
    if (!target)
        return HandlerResult::Fault(fault);
    if (!thread.condition_result && !set_instruction_pointer(thread, *target, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_terminate_this_script(ScriptThread& thread) {
    // PSP 002B7D38 / PS2 003B2040 take no parameters. They conditionally
    // clear the mission-running global from thread +0x217, transfer the
    // object active->idle, clear its +0x208 active byte and return literal 1.
    if (!script_termination_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0023RequiresScriptTerminationAdapter);
    script_termination_adapter_(*this, thread, thread.is_mission_script);
    thread.is_active = false;
    thread.terminated = true;
    return HandlerResult::Yield();
}

HandlerResult ScriptVm::handle_start_new_script(ScriptThread& thread) {
    // PSP forced-linear 002B7D84 and PS2 003B2098 both do exactly this:
    // CollectParameters(1) -> clamp script index at zero -> host allocation
    // -> CollectParameters(0x60) directly into child + 0x54 -> return 0.
    // +0x54 is the start of a freshly created script's local dword area.
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> encoded_script_index = read_parameter(thread, fault);
    if (!encoded_script_index)
        return HandlerResult::Fault(fault);
    if (!start_new_script_adapter_)
        return HandlerResult::Fault(VmFault::Opcode0024RequiresStartNewScriptAdapter);

    const std::int32_t script_index = *encoded_script_index < 0 ? 0 : *encoded_script_index;
    ScriptThread* const child = start_new_script_adapter_(*this, script_index);
    if (!child)
        return HandlerResult::Fault(VmFault::StartNewScriptAdapterReturnedNull);

    std::vector<std::int32_t> child_locals;
    if (!collect_parameters(thread, 0x60, child_locals, fault))
        return HandlerResult::Fault(fault);
    if (child->locals.size() < child_locals.size())
        child->locals.resize(child_locals.size());
    for (std::size_t index = 0; index < child_locals.size(); ++index)
        child->locals[index] = child_locals[index];
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_gosub(ScriptThread& thread) {
    // PSP 002B7E08 calls the confirmed three-argument collector with count 1.
    // PS2 003B2108 then pushes the post-parameter IP at thread+0x14 + 4*depth
    // with no function-frame tag and transfers to that target.
    if (thread.call_stack.size() >= max_call_depth_)
        return HandlerResult::Fault(VmFault::CallDepthExceeded);

    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> target = read_integer_parameter(thread, fault);
    if (!target)
        return HandlerResult::Fault(fault);

    const std::uint32_t return_ip = thread.ip;
    thread.call_stack.push_back({0, return_ip, 0, 0, 0, 0});
    if (!set_instruction_pointer(thread, *target, fault)) {
        thread.call_stack.pop_back();
        return HandlerResult::Fault(fault);
    }
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_gosub_file(ScriptThread& thread) {
    // PSP forced-linear 002B82F8 and PS2 003B23D8 call CollectParameters(2),
    // push the post-parameter IP as a tagless frame, then use only parameter 0
    // as the new script IP. Parameter 1 is consumed by the native VM but is
    // not read again by either handler; its file-level meaning is outside this
    // confirmed interpreter contract.
    if (thread.call_stack.size() >= max_call_depth_)
        return HandlerResult::Fault(VmFault::CallDepthExceeded);

    VmFault fault = VmFault::None;
    std::vector<std::int32_t> parameters;
    if (!collect_parameters(thread, 2, parameters, fault) || parameters.size() != 2)
        return HandlerResult::Fault(fault == VmFault::None ? VmFault::TruncatedBytecode : fault);

    const std::uint32_t return_ip = thread.ip;
    thread.call_stack.push_back({0, return_ip, 0, 0, 0, 0});
    if (!set_instruction_pointer(thread, parameters[0], fault)) {
        thread.call_stack.pop_back();
        return HandlerResult::Fault(fault);
    }
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_call(ScriptThread& thread, bool invert_return) {
    if (thread.call_stack.size() >= max_call_depth_)
        return HandlerResult::Fault(VmFault::CallDepthExceeded);

    const std::uint32_t call_header_ip = thread.ip;
    std::uint8_t input_count = 0;
    std::uint8_t output_count = 0;
    std::uint8_t locals_offset = 0;
    if (!script_space_.read_u8(thread.ip, input_count) ||
        !script_space_.read_u8(thread.ip, output_count) ||
        !script_space_.read_u8(thread.ip, locals_offset)) {
        return HandlerResult::Fault(VmFault::TruncatedBytecode);
    }

    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> target = read_integer_parameter(thread, fault);
    if (!target)
        return HandlerResult::Fault(fault);

    const std::size_t locals_start = thread.locals_pointer + locals_offset;
    if (locals_start > thread.locals.size() || input_count > thread.locals.size() - locals_start)
        return HandlerResult::Fault(VmFault::LocalStorageExceeded);

    for (std::size_t i = 0; i < input_count; ++i) {
        const std::optional<std::int32_t> value = read_integer_parameter(thread, fault);
        if (!value)
            return HandlerResult::Fault(fault);
        thread.locals[locals_start + i] = *value;
    }

    // The native frame packs the CALL header IP into bits 0..21, its CALL or
    // CALL_NOT tags into bits 22..23 and the byte distance from *after* the
    // three-byte header to the output destinations into bits 24..31.
    const std::uint32_t return_parameter_bytes = thread.ip - (call_header_ip + 3U);
    if (call_header_ip > 0x003FFFFFU || return_parameter_bytes > 0xFFU)
        return HandlerResult::Fault(VmFault::CallFrameEncodingOverflow);
    thread.call_stack.push_back({
        invert_return ? kCallNotTag : kCallTag,
        call_header_ip,
        static_cast<std::uint8_t>(return_parameter_bytes),
        input_count,
        output_count,
        locals_offset,
    });
    thread.locals_pointer = locals_start;
    thread.saved_ip = call_header_ip;
    if (!set_instruction_pointer(thread, *target, fault)) {
        thread.locals_pointer -= locals_offset;
        thread.call_stack.pop_back();
        return HandlerResult::Fault(fault);
    }
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_return(ScriptThread& thread) {
    if (thread.call_stack.empty())
        return HandlerResult::Fault(VmFault::ReturnWithoutCallFrame);

    const CallFrame frame = thread.call_stack.back();
    thread.call_stack.pop_back();

    // Native 0x0005F09C always restores the raw stack entry as IP first. A
    // tagless entry is the separately proved GOSUB frame and returns directly;
    // bits 22..23 select the richer CALL/CALL_NOT function-frame ABI below.
    if ((frame.tag & kCallTag) == 0) {
        thread.ip = frame.call_header_ip;
        return HandlerResult::Continue();
    }

    // PSP FUN_0005F09C conditionally flips thread+0x209 for CALL_NOT,
    // rereads the header, copies outputs from locals[LP + input_count], then
    // rolls back LP and decodes caller-side output destinations.
    if ((frame.tag & 0x00400000U) != 0)
        thread.condition_result = !thread.condition_result;

    thread.ip = frame.call_header_ip;
    std::uint8_t input_count = 0;
    std::uint8_t output_count = 0;
    std::uint8_t locals_offset = 0;
    if (!script_space_.read_u8(thread.ip, input_count) ||
        !script_space_.read_u8(thread.ip, output_count) ||
        !script_space_.read_u8(thread.ip, locals_offset)) {
        return HandlerResult::Fault(VmFault::TruncatedBytecode);
    }

    // Detect malformed/incompatible bytecode early. The native handler rereads
    // these bytes; equality is an invariant of this semantic frame model.
    if (input_count != frame.input_count || output_count != frame.output_count ||
        locals_offset != frame.locals_offset) {
        return HandlerResult::Fault(VmFault::InvalidVariableReference);
    }
    const std::size_t output_start = thread.locals_pointer + input_count;
    if (output_start > thread.locals.size() || output_count > thread.locals.size() - output_start)
        return HandlerResult::Fault(VmFault::LocalStorageExceeded);

    std::vector<std::int32_t> outputs;
    outputs.reserve(output_count);
    for (std::size_t index = 0; index < output_count; ++index)
        outputs.push_back(thread.locals[output_start + index]);

    const std::uint64_t output_destinations_ip =
        static_cast<std::uint64_t>(thread.ip) + frame.return_parameter_bytes;
    if (output_destinations_ip > std::numeric_limits<std::uint32_t>::max() ||
        output_destinations_ip > script_space_.size()) {
        return HandlerResult::Fault(VmFault::TruncatedBytecode);
    }
    thread.ip = static_cast<std::uint32_t>(output_destinations_ip);
    if (locals_offset > thread.locals_pointer)
        return HandlerResult::Fault(VmFault::LocalStorageExceeded);
    thread.locals_pointer -= locals_offset;

    VmFault fault = VmFault::None;
    if (!store_parameters(thread, outputs, fault))
        return HandlerResult::Fault(fault);
    return HandlerResult::Continue();
}

HandlerResult ScriptVm::handle_return_condition(ScriptThread& thread, bool base_condition) {
    // PSP 002B80D4/002B815C and PS2 003B21A8/003B2230 use precisely the
    // shared 055A reducer with a constant base predicate, then invoke the
    // regular tagless-or-function ReturnFromGosubOrFunction core.
    reduce_condition_result(thread, base_condition);
    return handle_return(thread);
}

HandlerResult ScriptVm::handle_if(ScriptThread& thread) {
    VmFault fault = VmFault::None;
    const std::optional<std::int32_t> count = read_integer_parameter(thread, fault);
    if (!count)
        return HandlerResult::Fault(fault);

    // PSP 002B81E0 / PS2 003B22B8 truncate the supplied count to uint16. A
    // nonzero count seeds an AND group with true (<9) or an OR group with
    // false (>=9); zero leaves the preceding aggregate untouched.
    thread.condition_state = static_cast<std::uint16_t>(*count);
    if (thread.condition_state != 0)
        thread.condition_result = thread.condition_state < 9U;
    return HandlerResult::Continue();
}

void ScriptVm::reduce_condition_result(ScriptThread& thread, bool base_condition) {
    // PSP 0x0009E4C8 and PS2 0x0012C1E0 both derive the per-command result by
    // XORing the engine predicate with the dispatcher's raw opcode NOT bit.
    const bool condition = base_condition != thread.not_flag;
    const std::uint16_t previous_state = thread.condition_state;
    if (previous_state == 0) {
        thread.condition_result = condition;
        return;
    }

    thread.condition_state = static_cast<std::uint16_t>(previous_state - 1U);
    if (previous_state < 9U) {
        thread.condition_result = condition && thread.condition_result;
        return;
    }

    thread.condition_result = condition || thread.condition_result;
    if (thread.condition_state < 0x15U)
        thread.condition_state = 0;
}

HandlerResult ScriptVm::handle_055a(ScriptThread& thread) {
    if (!opcode_055a_condition_probe_)
        return HandlerResult::Fault(VmFault::Opcode055ARequiresConditionProbe);
    reduce_condition_result(thread, opcode_055a_condition_probe_(*this, thread));
    return HandlerResult::Continue();
}

} // namespace vcs::vm
