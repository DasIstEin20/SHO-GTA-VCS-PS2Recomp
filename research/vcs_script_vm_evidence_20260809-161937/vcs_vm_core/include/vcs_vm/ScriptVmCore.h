#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace vcs::vm {

// The confirmed VCS PSP table ends at 0x055A.  Slots exist independently of
// whether their semantics have been recovered.
constexpr std::uint16_t kLastPspOpcode = 0x055A;
constexpr std::size_t kPspOpcodeSlots = static_cast<std::size_t>(kLastPspOpcode) + 1;

constexpr std::uint16_t kOpcodeNotBit = 0x8000;
constexpr std::uint16_t kOpcodeMask = 0x7FFF;

enum class NativeHandlerStatus : std::uint8_t {
    Continue = 0,
    Yield = 1,
};

enum class VmFault : std::uint8_t {
    None,
    InstructionPointerOutOfRange,
    UnsupportedOpcode,
    UnsupportedParameterEncoding,
    TruncatedBytecode,
    InvalidScriptTarget,
    CallDepthExceeded,
    LocalStorageExceeded,
    InvalidVariableReference,
    InvalidArrayIndex,
    VariableStorageOutOfRange,
    IntegerDivisionByZero,
    IntegerDivisionOverflow,
    FloatToIntegerConversionOutOfRange,
    InvalidScriptStringAddress,
    TimedArithmeticRequiresTimeStep,
    Opcode0045RequiresConditionProbe,
    Opcode007FRequiresButtonPressedProbe,
    Opcode00C8RequiresFadeColourAdapter,
    Opcode00C9RequiresFadeStartAdapter,
    Opcode00CARequiresFadeStatusProbe,
    Opcode0049RequiresVehicleDeleteAdapter,
    Opcode0113RequiresVehicleReleaseMarkAdapter,
    Opcode04E5RequiresGlobalByteStoreAdapter,
    Opcode0107RequiresPlayerControlAdapter,
    Opcode0044RequiresCharacterCoordinateAdapter,
    Opcode0023RequiresScriptTerminationAdapter,
    Opcode013FRequiresPickupModelObjectResolver,
    Opcode013FRequiresPickupGroundHeightProbe,
    Opcode013FRequiresPickupCreateAdapter,
    Opcode01E7RequiresUniqueJumpsTotalAdapter,
    Opcode00A5RequiresPlayerStateCodeTwoProbe,
    Opcode0059RequiresClearPrintsAdapter,
    Opcode009FRequiresCharacterDeadProbe,
    Opcode0289RequiresMissionStreamingSubsystem,
    Opcode0203RequiresDisplayTextAdapter,
    Opcode0268RequiresClearHelpAdapter,
    Opcode0401RequiresPauseMenuToggleAdapter,
    Opcode0377RequiresPadStateAdapter,
    Opcode0079RequiresMissionLaunchAdapter,
    MissionLaunchAdapterReturnedNull,
    Opcode0024RequiresStartNewScriptAdapter,
    StartNewScriptAdapterReturnedNull,
    ReturnWithoutCallFrame,
    CallFrameEncodingOverflow,
    Opcode055ARequiresConditionProbe,
    InstructionBudgetExhausted,
};

// A semantic reference returned by the confirmed VCS PSP variable resolver
// (FUN_0005DA7C). Script-space variables are byte offsets into ScriptSpace;
// thread-local variables are int32 slot indices.
struct ScriptVariableRef {
    enum class Storage : std::uint8_t {
        ThreadLocal,
        ScriptSpace,
    };

    Storage storage = Storage::ThreadLocal;
    std::uint32_t index = 0;
};

struct HandlerResult {
    NativeHandlerStatus status = NativeHandlerStatus::Continue;
    VmFault fault = VmFault::None;

    static HandlerResult Continue() { return {}; }
    static HandlerResult Yield() { return {NativeHandlerStatus::Yield, VmFault::None}; }
    static HandlerResult Fault(VmFault value) { return {NativeHandlerStatus::Yield, value}; }
};

struct CallFrame {
    // The known VCS CALL/CALL_NOT ABI tags.  They are deliberately preserved
    // separately from the portable C++ representation of a frame.
    std::uint32_t tag = 0;
    std::uint32_t call_header_ip = 0;
    // Exact high-byte payload of the native function-return frame: bytes from
    // immediately after the 3-byte CALL header to the output destinations.
    std::uint8_t return_parameter_bytes = 0;
    std::uint8_t input_count = 0;
    std::uint8_t output_count = 0;
    std::uint8_t locals_offset = 0;
};

struct ScriptThread {
    // Semantic fields proved in the VCS PSP/PS2 VM.  This is intentionally not
    // a binary overlay for either platform's CRunningScript layout.
    std::uint32_t ip = 0;
    std::uint32_t saved_ip = 0;
    std::uint32_t wake_time = 0;
    // Portable semantic analogue of the monotonic value written at native
    // +0x08 by CTheScripts::StartNewScript before that counter increments.
    // It is scheduler-owned, not interpreted by the VM core itself.
    std::uint32_t scheduler_id = 0;
    bool wait_flag = false;
    // Semantic analogue of the target script's native +0x20A launch flag.
    // The portable core does not assert the original member's source name.
    bool mission_launch_requested = false;
    // Portable semantic name for native byte +0x217. Opcode 0289 sets it on a
    // streamed target and opcode 04F2 queries it through the condition VM;
    // this is not asserted as the original source-field spelling.
    bool is_mission_script = false;
    // Semantic analogue of native byte +0x208, cleared by opcode 0023 after
    // the host has moved the thread from the active to idle scheduler list.
    bool is_active = true;
    bool terminated = false;
    // Exact portable analogue of the native eight-byte field at +0x20F set by
    // opcode 0238. It is not a NUL-terminated std::string: a source of eight
    // non-NUL bytes fills the entire field.
    std::array<char, 8> script_name{};
    bool not_flag = false;
    std::uint16_t condition_state = 0;
    bool condition_result = false;

    std::vector<std::int32_t> locals;
    std::size_t locals_pointer = 0;
    std::vector<CallFrame> call_stack;
};

struct ProcessResult {
    NativeHandlerStatus status = NativeHandlerStatus::Continue;
    VmFault fault = VmFault::None;
    std::uint16_t last_opcode = 0;
    std::size_t executed = 0;
};

class ScriptSpace {
public:
    explicit ScriptSpace(std::vector<std::uint8_t> bytes, std::uint32_t main_script_size = 0);

    std::uint32_t size() const;
    std::uint32_t main_script_size() const;
    bool is_valid_ip(std::uint32_t ip) const;
    bool read_u8(std::uint32_t& ip, std::uint8_t& value) const;
    bool read_i8(std::uint32_t& ip, std::int8_t& value) const;
    bool read_i16(std::uint32_t& ip, std::int16_t& value) const;
    bool read_i32(std::uint32_t& ip, std::int32_t& value) const;
    bool read_i32_at(std::uint32_t address, std::int32_t& value) const;
    bool write_i32_at(std::uint32_t address, std::int32_t value);
    // Bounded byte transfer for an owned ScriptSpace staging buffer.  This is
    // intentionally a memory primitive, not a mission-launch adapter: the
    // caller must still prove source ownership, range and lifecycle.
    bool write_bytes_at(std::uint32_t address, const std::vector<std::uint8_t>& bytes);

private:
    std::vector<std::uint8_t> bytes_;
    std::uint32_t main_script_size_;
};

class ScriptVm;
using OpcodeHandler = std::function<HandlerResult(ScriptVm&, ScriptThread&, std::uint16_t)>;
// 0x055A's engine-specific predicate is intentionally supplied by the host.
// It is the predicate *before* the raw SCM opcode NOT bit is applied.  The
// VCS condition reducer itself is implemented by ScriptVm from PSP/PS2 proof.
using Opcode055AConditionProbe = std::function<bool(ScriptVm&, ScriptThread&)>;
// Opcode 0024 creates a real engine-managed script object, then the VM copies
// 96 decoded local values into it. The host owns its allocation/lifetime and
// must return a child already initialized at the requested script entry.
using StartNewScriptAdapter = std::function<ScriptThread*(ScriptVm&, std::int32_t script_index)>;
// Opcodes 0031..0034 multiply their one collected binary32 value by the
// engine frame-time scalar. It is an engine-owned value, so the portable VM
// requires an explicit host provider instead of embedding a guessed timer.
using TimeStepProvider = std::function<float()>;
// Opcode 0045 owns parameter decoding and the VM condition reducer, while its
// character-pool/state predicate belongs to the game engine.
using Opcode0045ConditionProbe = std::function<bool(ScriptVm&, ScriptThread&, std::int32_t char_handle)>;
// Opcode 00A5 resolves one player selector, compares the recovered native
// state byte with 2, then applies raw NOT and the shared condition reducer.
// The meaning of state value 2 stays with the host.
using Opcode00A5PlayerStateCodeTwoProbe = std::function<bool(ScriptVm&, ScriptThread&, std::int32_t player_selector)>;
// Opcode 0059 has no SCM parameters and enters a single native UI clear path.
// The owning display/print subsystem is intentionally host-owned.
using ClearPrintsAdapter = std::function<void()>;
// Opcode 009F has a multi-branch engine predicate over a resolved character
// object (including a null-handle true case) before raw NOT/reduction.
using Opcode009FCharacterDeadProbe = std::function<bool(ScriptVm&, ScriptThread&, std::int32_t character_handle)>;
// Opcode 007F converts each of two collected raw parameters to uint16, asks an
// engine input helper for a predicate, then enters the common condition VM.
// The two native argument meanings are intentionally host-owned.
using Opcode007FButtonPressedProbe = std::function<bool(ScriptVm&, ScriptThread&, std::uint16_t first, std::uint16_t second)>;
using FadeSetColourAdapter = std::function<void(std::uint8_t red, std::uint8_t green, std::uint8_t blue)>;
using FadeStartAdapter = std::function<void(float duration_seconds, std::int16_t mode)>;
using FadeStatusProbe = std::function<bool()>;
// Opcode 0203 collects two raw binary32 coordinate payloads and then a
// separate raw text-key/offset parameter. GXT lookup, UTF-16 buffering and
// rendering are native text-manager work and remain with the host.
using DisplayTextAdapter = std::function<void(std::int32_t offset_left_raw,
                                              std::int32_t offset_top_raw,
                                              std::int32_t text_key_raw)>;
// Opcode 0268 invokes two native help/HUD clearing paths with no SCM
// parameters. Their manager allocation/state remains host-owned.
using ClearHelpAdapter = std::function<void()>;
// Opcode 0049 collects one raw vehicle handle.  The native handler resolves it
// through an engine-owned vehicle pool, conditionally performs the observed
// +0x20A cleanup path, then removes the entity and its references.  Pool
// lookup, cleanup-list ownership and entity lifetime stay with the host.
using VehicleDeleteAdapter = std::function<void(std::int32_t vehicle_handle,
                                                bool native_context_flag)>;
// Opcode 0113 collects one raw vehicle handle, resolves it through the native
// vehicle-pool path and invokes a distinct release-mark helper.  When native
// thread byte +0x20A is nonzero it also takes a separate context-cleanup path.
// Neither helper's original source spelling nor the release-policy data model
// is inferred by the portable VM.
using VehicleReleaseMarkAdapter = std::function<void(std::int32_t vehicle_handle,
                                                     bool native_context_flag)>;
// Opcode 04E5 writes the low byte of its second collected raw dword into an
// engine-owned byte region selected by its first raw dword. The native global
// region's source name and lifetime have not been recovered.
using GlobalByteStoreAdapter = std::function<void(std::int32_t slot_offset,
                                                  std::uint8_t value)>;
// Opcode 0107 resolves its first raw parameter through the player table and
// branches on only whether its second raw parameter is zero. Its player safety,
// camera and reset details are engine-owned.
using PlayerControlAdapter = std::function<void(std::int32_t player_selector,
                                                bool enabled)>;
// Opcode 0044 passes one character handle and three binary32 coordinate
// payloads to the host. Native z == -100.0f or z <= -250.0f selects a ground
// height query before the actual placement path.
using CharacterCoordinateAdapter = std::function<void(std::int32_t character_handle,
                                                       std::int32_t x_raw,
                                                       std::int32_t y_raw,
                                                       std::int32_t z_raw,
                                                       bool resolve_ground_height)>;
// Opcode 013F first maps a negative model-object selector through an
// engine-owned 28-byte record table. The portable core owns the observed
// branch, while the table's lifetime and record meaning remain host-owned.
using PickupModelObjectResolver = std::function<std::optional<std::int32_t>(std::int32_t negative_selector)>;
// For native Z == -100.0f or Z <= -250.0f, opcode 013F obtains a ground
// height from the world and passes height + 0.5f to its pickup constructor.
using PickupGroundHeightProbe = std::function<std::optional<float>(float x, float y)>;
// The pickup manager/pool and returned handle encoding are engine-owned. The
// core has already resolved the model-object form and ground-Z sentinel.
using PickupCreateAdapter = std::function<std::int32_t(std::int32_t model_id,
                                                        std::uint8_t pickup_type,
                                                        std::int32_t ammo,
                                                        float x,
                                                        float y,
                                                        float z)>;
// Opcode 01E7 writes its one collected raw int32 to the game-owned aggregate
// statistic identified by direct PSP/PS2 and reLCS source evidence as total
// unique jumps. Storage, save-game persistence and presentation are host work.
using UniqueJumpsTotalAdapter = std::function<void(std::int32_t total)>;
// Opcode 0401 normalizes one collected raw parameter to `parameter != 0` and
// forwards that boolean to the native pause-menu object. The host owns the
// menu-object lifetime and its platform-specific application.
using PauseMenuToggleAdapter = std::function<void(bool parameter_is_nonzero)>;
// Selector 0..44 maps to distinct native pad accessors. The host owns their
// device/input meaning and returns the raw int32 result for that selector.
using PadStateAdapter = std::function<std::int32_t(std::uint32_t selector)>;
// Opcode 0079's native resolver returns a target running-script object, whose
// +0x20A flag is then set. The host owns lookup/lifetime of that target.
using MissionLaunchAdapter = std::function<ScriptThread*(ScriptVm&, std::int32_t mission_id)>;
// Opcode 0023 clears a global mission-running byte only for a marked thread,
// then removes that thread from the active list and adds it to the idle list.
// The portable VM records its local active/terminated state; host list/global
// ownership and the ordering of that transfer remain explicit here.
using ScriptTerminationAdapter = std::function<void(ScriptVm&, ScriptThread&, bool was_mission_script)>;

class ScriptVm {
public:
    explicit ScriptVm(ScriptSpace script_space, std::size_t max_call_depth = 16);

    ScriptSpace& script_space();
    const ScriptSpace& script_space() const;

    void set_clock(std::function<std::uint32_t()> clock);
    void set_opcode_055a_condition_probe(Opcode055AConditionProbe probe);
    void set_start_new_script_adapter(StartNewScriptAdapter adapter);
    void set_time_step_provider(TimeStepProvider provider);
    void set_opcode_0045_condition_probe(Opcode0045ConditionProbe probe);
    void set_opcode_00a5_player_state_code_two_probe(Opcode00A5PlayerStateCodeTwoProbe probe);
    void set_clear_prints_adapter(ClearPrintsAdapter adapter);
    void set_opcode_009f_character_dead_probe(Opcode009FCharacterDeadProbe probe);
    void set_opcode_007f_button_pressed_probe(Opcode007FButtonPressedProbe probe);
    void set_fade_set_colour_adapter(FadeSetColourAdapter adapter);
    void set_fade_start_adapter(FadeStartAdapter adapter);
    void set_fade_status_probe(FadeStatusProbe probe);
    void set_vehicle_delete_adapter(VehicleDeleteAdapter adapter);
    void set_vehicle_release_mark_adapter(VehicleReleaseMarkAdapter adapter);
    void set_global_byte_store_adapter(GlobalByteStoreAdapter adapter);
    void set_player_control_adapter(PlayerControlAdapter adapter);
    void set_character_coordinate_adapter(CharacterCoordinateAdapter adapter);
    void set_pickup_model_object_resolver(PickupModelObjectResolver resolver);
    void set_pickup_ground_height_probe(PickupGroundHeightProbe probe);
    void set_pickup_create_adapter(PickupCreateAdapter adapter);
    void set_unique_jumps_total_adapter(UniqueJumpsTotalAdapter adapter);
    void set_display_text_adapter(DisplayTextAdapter adapter);
    void set_clear_help_adapter(ClearHelpAdapter adapter);
    void set_pause_menu_toggle_adapter(PauseMenuToggleAdapter adapter);
    void set_pad_state_adapter(PadStateAdapter adapter);
    void set_mission_launch_adapter(MissionLaunchAdapter adapter);
    void set_script_termination_adapter(ScriptTerminationAdapter adapter);
    void register_handler(std::uint16_t opcode, OpcodeHandler handler);

    ProcessResult process_one_command(ScriptThread& thread);
    ProcessResult process(ScriptThread& thread, std::size_t instruction_budget = 10000);

    // Exact semantic primitives reconstructed from the VCS PSP decoder:
    //   CollectParameters                         (loop in this portable model)
    //   CollectNextParameterWithoutIncreasingPC   (0x0005E71C)
    //   GetPointerToScriptVariable                (0x0005DA7C)
    //   StoreParameters                           (0x0005E890)
    // Values are raw int32 payloads: float encodings intentionally preserve
    // their bit pattern rather than performing a C++ numeric conversion.
    std::optional<std::int32_t> collect_next_parameter_without_increasing_pc(
        ScriptThread& thread, std::uint32_t parameter_ip, VmFault& fault);
    // Consuming one-parameter convenience with the plural collector's VCS
    // tag-0 zero semantics. Use collect_next_parameter_without_increasing_pc
    // when modelling its distinct standalone decoder behaviour.
    std::optional<std::int32_t> read_parameter(ScriptThread& thread, VmFault& fault);
    bool collect_parameters(ScriptThread& thread, std::int16_t total,
                            std::vector<std::int32_t>& parameters, VmFault& fault);
    std::optional<ScriptVariableRef> get_pointer_to_script_variable(
        ScriptThread& thread, std::uint32_t& ip, VmFault& fault);
    bool read_script_variable(const ScriptThread& thread, const ScriptVariableRef& variable,
                              std::int32_t& value, VmFault& fault) const;
    bool write_script_variable(ScriptThread& thread, const ScriptVariableRef& variable,
                               std::int32_t value, VmFault& fault);
    bool store_parameters(ScriptThread& thread, const std::vector<std::int32_t>& parameters,
                          VmFault& fault);

    // Exact shared PSP/PS2 aggregation sequence used by opcode 0x055A.  The
    // supplied value is the underlying engine predicate, before NOT handling.
    void reduce_condition_result(ScriptThread& thread, bool base_condition);

    // Compatibility name retained for the initial five handlers. It now
    // delegates to the complete raw-int32 parameter decoder above.
    std::optional<std::int32_t> read_integer_parameter(ScriptThread& thread, VmFault& fault);
    bool set_instruction_pointer(ScriptThread& thread, std::int32_t encoded_target, VmFault& fault);

private:
    HandlerResult handle_nop(ScriptThread& thread);
    HandlerResult handle_wait(ScriptThread& thread);
    HandlerResult handle_goto(ScriptThread& thread);
    HandlerResult handle_assign(ScriptThread& thread);
    HandlerResult handle_integer_arithmetic(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_float_arithmetic(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_timed_float_arithmetic(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_numeric_conversion(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_absolute_value(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_integral_or_raw_equality_condition(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_float_condition(ScriptThread& thread, std::uint16_t opcode);
    HandlerResult handle_0045_is_char_still_alive(ScriptThread& thread);
    HandlerResult handle_00a5_player_state_code_two_condition(ScriptThread& thread);
    HandlerResult handle_clear_prints(ScriptThread& thread);
    HandlerResult handle_009f_character_dead_condition(ScriptThread& thread);
    HandlerResult handle_007f_is_button_pressed(ScriptThread& thread);
    HandlerResult handle_set_fading_colour(ScriptThread& thread);
    HandlerResult handle_do_fade(ScriptThread& thread);
    HandlerResult handle_get_fading_status(ScriptThread& thread);
    HandlerResult handle_delete_vehicle(ScriptThread& thread);
    HandlerResult handle_vehicle_release_mark(ScriptThread& thread);
    HandlerResult handle_global_byte_store(ScriptThread& thread);
    HandlerResult handle_player_control(ScriptThread& thread);
    HandlerResult handle_character_coordinate_placement(ScriptThread& thread);
    HandlerResult handle_create_pickup(ScriptThread& thread, bool has_ammo_parameter);
    HandlerResult handle_set_unique_jumps_total(ScriptThread& thread);
    HandlerResult handle_display_text(ScriptThread& thread);
    HandlerResult handle_clear_help(ScriptThread& thread);
    HandlerResult handle_pause_menu_toggle(ScriptThread& thread);
    HandlerResult handle_get_game_timer(ScriptThread& thread);
    HandlerResult handle_get_pad_state(ScriptThread& thread);
    HandlerResult handle_launch_mission(ScriptThread& thread);
    HandlerResult handle_script_name(ScriptThread& thread);
    HandlerResult handle_mission_streaming_boundary(ScriptThread& thread);
    HandlerResult handle_is_this_script_a_mission(ScriptThread& thread);
    HandlerResult handle_goto_if_false(ScriptThread& thread);
    HandlerResult handle_terminate_this_script(ScriptThread& thread);
    HandlerResult handle_start_new_script(ScriptThread& thread);
    HandlerResult handle_gosub(ScriptThread& thread);
    HandlerResult handle_gosub_file(ScriptThread& thread);
    HandlerResult handle_call(ScriptThread& thread, bool invert_return);
    HandlerResult handle_return(ScriptThread& thread);
    HandlerResult handle_return_condition(ScriptThread& thread, bool base_condition);
    HandlerResult handle_if(ScriptThread& thread);
    HandlerResult handle_055a(ScriptThread& thread);
    std::optional<std::int32_t> read_parameter_at(ScriptThread& thread, std::uint32_t& ip,
                                                   VmFault& fault);
    bool local_slot_is_valid(const ScriptThread& thread, std::uint32_t index) const;

    ScriptSpace script_space_;
    std::array<OpcodeHandler, kPspOpcodeSlots> handlers_{};
    std::function<std::uint32_t()> clock_;
    Opcode055AConditionProbe opcode_055a_condition_probe_;
    StartNewScriptAdapter start_new_script_adapter_;
    TimeStepProvider time_step_provider_;
    Opcode0045ConditionProbe opcode_0045_condition_probe_;
    Opcode00A5PlayerStateCodeTwoProbe opcode_00a5_player_state_code_two_probe_;
    ClearPrintsAdapter clear_prints_adapter_;
    Opcode009FCharacterDeadProbe opcode_009f_character_dead_probe_;
    Opcode007FButtonPressedProbe opcode_007f_button_pressed_probe_;
    FadeSetColourAdapter fade_set_colour_adapter_;
    FadeStartAdapter fade_start_adapter_;
    FadeStatusProbe fade_status_probe_;
    VehicleDeleteAdapter vehicle_delete_adapter_;
    VehicleReleaseMarkAdapter vehicle_release_mark_adapter_;
    GlobalByteStoreAdapter global_byte_store_adapter_;
    PlayerControlAdapter player_control_adapter_;
    CharacterCoordinateAdapter character_coordinate_adapter_;
    PickupModelObjectResolver pickup_model_object_resolver_;
    PickupGroundHeightProbe pickup_ground_height_probe_;
    PickupCreateAdapter pickup_create_adapter_;
    UniqueJumpsTotalAdapter unique_jumps_total_adapter_;
    DisplayTextAdapter display_text_adapter_;
    ClearHelpAdapter clear_help_adapter_;
    PauseMenuToggleAdapter pause_menu_toggle_adapter_;
    PadStateAdapter pad_state_adapter_;
    MissionLaunchAdapter mission_launch_adapter_;
    ScriptTerminationAdapter script_termination_adapter_;
    std::size_t max_call_depth_;
};

} // namespace vcs::vm
