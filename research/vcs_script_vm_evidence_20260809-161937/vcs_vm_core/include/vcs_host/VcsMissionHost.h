#pragma once

#include "vcs_host/VcsDiscFs.h"
#include "vcs_vm/ScriptVmCore.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace vcs::host {

struct VcsMissionMetadata {
    std::uint32_t variable_space_size = 0;
    std::uint32_t object_size = 0;
    std::uint16_t num_true_globals = 0;
    std::uint16_t most_globals = 0;
    std::uint32_t largest_mission_script_size = 0;
    std::int16_t mission_script_count = 0;
    std::uint16_t exclusive_mission_count = 0;
    std::vector<std::uint32_t> offsets;
};

// The eight-byte VCS SCM header and the post-header ScriptSpace layout needed
// by ReadMultiScriptFileOffsetsFromScript. This remains a script container,
// not an assertion about the broader IMG/streaming format.
class VcsScriptContainer {
public:
    static std::optional<VcsScriptContainer> parse(std::vector<std::uint8_t> file_bytes,
                                                    std::string& error);

    const std::vector<std::uint8_t>& source_body() const;
    std::uint32_t main_script_size() const;
    std::uint32_t largest_mission_script_size() const;
    const VcsMissionMetadata& mission_metadata() const;
    std::optional<std::uint32_t> find_main_entry() const;
    // Structural identification only: this is intentionally not a content
    // hash or a licensing check. It lets a reproducible bootstrap distinguish
    // the locally evidenced SLES_546.22 SCM layout from another compatible
    // Stories container without rejecting future research builds by default.
    bool matches_known_vcs_ps2_sles_546_22_layout() const;
    std::string layout_profile() const;

private:
    std::vector<std::uint8_t> source_body_;
    std::uint32_t main_script_size_ = 0;
    std::uint32_t largest_mission_script_size_ = 0;
    VcsMissionMetadata metadata_;
};

struct MissionLaunchSnapshot {
    bool mission_active = false;
    std::uint32_t stream_scope_depth = 0;
    std::uint32_t next_process_id = 0;
    std::size_t idle_thread_count = 0;
    std::size_t active_thread_count = 0;
    std::optional<std::int32_t> last_selector;
    std::optional<std::uint32_t> last_child_scheduler_id;
    std::optional<vcs::vm::ProcessResult> last_child_result;
};

// The host portion of native opcode 0289. It retains the recovered sequence:
// offset range -> scoped source positioning/copy -> StartNewScript factory ->
// scope finalization -> marker/global writes -> re-entrant Process(child).
// It intentionally does not implement model, world, rendering or other game
// subsystems; a child reaching one of those adapters exits with the core's
// precise fault instead of fabricating successful game state.
class IVcsMissionHost {
public:
    virtual ~IVcsMissionHost() = default;

    virtual void bind(vcs::vm::ScriptVm& vm) = 0;
    virtual MissionLaunchSnapshot snapshot() const = 0;
};

class VcsMissionHost final : public IVcsMissionHost {
public:
    explicit VcsMissionHost(VcsScriptContainer container,
                            std::size_t thread_pool_size = 64,
                            std::size_t child_instruction_budget = 10000);

    void bind(vcs::vm::ScriptVm& vm) override;
    MissionLaunchSnapshot snapshot() const override;

    const VcsScriptContainer& container() const;
    void set_mission_mode(std::int32_t mode);

private:
    vcs::vm::HandlerResult handle_opcode_0289(vcs::vm::ScriptVm& vm,
                                               vcs::vm::ScriptThread& parent);
    vcs::vm::ScriptThread* start_new_script(std::uint32_t entry_ip);
    void enter_stream_scope();
    void leave_stream_scope();

    VcsScriptContainer container_;
    std::vector<vcs::vm::ScriptThread> thread_pool_;
    std::vector<std::size_t> idle_indices_;
    std::vector<std::size_t> active_indices_;
    std::int32_t mission_mode_ = 0;
    std::uint32_t next_process_id_ = 0;
    std::uint32_t stream_scope_depth_ = 0;
    bool mission_active_ = false;
    std::size_t child_instruction_budget_ = 10000;
    std::optional<std::int32_t> last_selector_;
    std::optional<std::uint32_t> last_child_scheduler_id_;
    std::optional<vcs::vm::ProcessResult> last_child_result_;
};

// Opens one named SCM from a read-only game source and parses the exact
// script-container fields above. The caller owns version validation policy.
std::optional<VcsScriptContainer> load_vcs_script_container(const IVcsFileSystem& fs,
                                                             const std::string& scm_path,
                                                             std::string& error);

} // namespace vcs::host
