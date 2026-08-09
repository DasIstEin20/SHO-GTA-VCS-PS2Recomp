#include "vcs_host/VcsMissionHost.h"

#include <algorithm>
#include <array>
#include <limits>

namespace vcs::host {
namespace {

constexpr std::array<std::uint8_t, 8> kMainEntryMarker{
    0x38, 0x02, 0x0A, 'M', 'A', 'I', 'N', 0x00,
};
constexpr std::size_t kPortableLocalSlots = 106;

bool read_u16_le(const std::vector<std::uint8_t>& bytes, std::size_t offset,
                 std::uint16_t& value) {
    if (offset > bytes.size() || bytes.size() - offset < 2)
        return false;
    value = static_cast<std::uint16_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
    return true;
}

bool read_i16_le(const std::vector<std::uint8_t>& bytes, std::size_t offset,
                 std::int16_t& value) {
    std::uint16_t raw = 0;
    if (!read_u16_le(bytes, offset, raw))
        return false;
    value = static_cast<std::int16_t>(raw);
    return true;
}

bool read_u32_le(const std::vector<std::uint8_t>& bytes, std::size_t offset,
                 std::uint32_t& value) {
    if (offset > bytes.size() || bytes.size() - offset < 4)
        return false;
    value = static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
    return true;
}

} // namespace

std::optional<VcsScriptContainer> VcsScriptContainer::parse(std::vector<std::uint8_t> file_bytes,
                                                             std::string& error) {
    if (file_bytes.size() < 8) {
        error = "VCS SCM is shorter than its eight-byte header";
        return std::nullopt;
    }
    VcsScriptContainer container;
    if (!read_u32_le(file_bytes, 0, container.main_script_size_) ||
        !read_u32_le(file_bytes, 4, container.largest_mission_script_size_)) {
        error = "could not parse VCS SCM header";
        return std::nullopt;
    }
    container.source_body_.assign(file_bytes.begin() + 8, file_bytes.end());
    if (container.main_script_size_ == 0 ||
        container.main_script_size_ > container.source_body_.size() ||
        container.largest_mission_script_size_ == 0) {
        error = "VCS SCM header has an invalid main/mission size";
        return std::nullopt;
    }

    VcsMissionMetadata& metadata = container.metadata_;
    if (!read_u32_le(container.source_body_, 3, metadata.variable_space_size) ||
        metadata.variable_space_size > container.source_body_.size() - 3) {
        error = "could not read bounded ScriptSpace variable-space size";
        return std::nullopt;
    }
    const std::size_t object_size_address = static_cast<std::size_t>(metadata.variable_space_size) + 3;
    if (!read_u32_le(container.source_body_, object_size_address, metadata.object_size) ||
        metadata.object_size > container.source_body_.size() - 8) {
        error = "could not read bounded VCS script-object size";
        return std::nullopt;
    }
    const std::size_t fields = static_cast<std::size_t>(metadata.object_size) + 8;
    if (!read_u16_le(container.source_body_, fields, metadata.num_true_globals) ||
        !read_u16_le(container.source_body_, fields + 2, metadata.most_globals) ||
        !read_u32_le(container.source_body_, fields + 4, metadata.largest_mission_script_size) ||
        !read_i16_le(container.source_body_, fields + 8, metadata.mission_script_count) ||
        !read_u16_le(container.source_body_, fields + 10, metadata.exclusive_mission_count)) {
        error = "could not read bounded VCS mission metadata";
        return std::nullopt;
    }
    if (metadata.largest_mission_script_size != container.largest_mission_script_size_ ||
        metadata.mission_script_count <= 0) {
        error = "VCS SCM header and mission metadata disagree";
        return std::nullopt;
    }
    const std::size_t count = static_cast<std::size_t>(metadata.mission_script_count);
    const std::size_t table = fields + 12;
    if (table > container.source_body_.size() || count > (container.source_body_.size() - table) / 4) {
        error = "VCS mission offset table exceeds ScriptSpace";
        return std::nullopt;
    }
    metadata.offsets.resize(count);
    for (std::size_t index = 0; index < count; ++index) {
        if (!read_u32_le(container.source_body_, table + index * 4, metadata.offsets[index])) {
            error = "could not read VCS mission offset table entry";
            return std::nullopt;
        }
    }
    return container;
}

const std::vector<std::uint8_t>& VcsScriptContainer::source_body() const { return source_body_; }
std::uint32_t VcsScriptContainer::main_script_size() const { return main_script_size_; }
std::uint32_t VcsScriptContainer::largest_mission_script_size() const { return largest_mission_script_size_; }
const VcsMissionMetadata& VcsScriptContainer::mission_metadata() const { return metadata_; }

std::optional<std::uint32_t> VcsScriptContainer::find_main_entry() const {
    if (main_script_size_ > source_body_.size())
        return std::nullopt;
    const auto begin = source_body_.begin();
    const auto end = begin + static_cast<std::ptrdiff_t>(main_script_size_);
    const auto found = std::search(begin, end, kMainEntryMarker.begin(), kMainEntryMarker.end());
    if (found == end || std::search(found + 1, end, kMainEntryMarker.begin(), kMainEntryMarker.end()) != end)
        return std::nullopt;
    return static_cast<std::uint32_t>(std::distance(begin, found));
}

bool VcsScriptContainer::matches_known_vcs_ps2_sles_546_22_layout() const {
    // Read directly from the supplied local SLES_546.22-extracted MAIN.SCM;
    // this recognizes one container layout, not a cryptographic disc identity.
    return main_script_size_ == 0x0003CAE3U &&
        largest_mission_script_size_ == 0x000096A9U &&
        metadata_.variable_space_size == 0x000063F4U &&
        metadata_.object_size == 0x00007664U &&
        metadata_.mission_script_count == 101;
}

std::string VcsScriptContainer::layout_profile() const {
    if (matches_known_vcs_ps2_sles_546_22_layout())
        return "VCS_PS2_SLES_546.22_LAYOUT";
    return "COMPATIBLE_VCS_SCM_LAYOUT_UNVERIFIED";
}

VcsMissionHost::VcsMissionHost(VcsScriptContainer container, std::size_t thread_pool_size,
                               std::size_t child_instruction_budget)
    : container_(std::move(container)),
      thread_pool_(std::max<std::size_t>(thread_pool_size, 1)),
      child_instruction_budget_(std::max<std::size_t>(child_instruction_budget, 1)) {
    idle_indices_.reserve(thread_pool_.size());
    active_indices_.reserve(thread_pool_.size());
    for (std::size_t index = 0; index < thread_pool_.size(); ++index)
        idle_indices_.push_back(thread_pool_.size() - index - 1);
}

void VcsMissionHost::bind(vcs::vm::ScriptVm& vm) {
    vm.register_handler(0x0289, [this](vcs::vm::ScriptVm& inner_vm, vcs::vm::ScriptThread& parent,
                                       std::uint16_t) {
        return handle_opcode_0289(inner_vm, parent);
    });
}

MissionLaunchSnapshot VcsMissionHost::snapshot() const {
    return {mission_active_, stream_scope_depth_, next_process_id_, idle_indices_.size(),
            active_indices_.size(), last_selector_, last_child_scheduler_id_, last_child_result_};
}

const VcsScriptContainer& VcsMissionHost::container() const { return container_; }
void VcsMissionHost::set_mission_mode(std::int32_t mode) { mission_mode_ = mode; }

void VcsMissionHost::enter_stream_scope() { ++stream_scope_depth_; }

void VcsMissionHost::leave_stream_scope() {
    // The native finalizer is entered only when this decrement reaches zero.
    // There is no extra finalizer state to invent in this narrow host yet.
    if (stream_scope_depth_ != 0)
        --stream_scope_depth_;
}

vcs::vm::ScriptThread* VcsMissionHost::start_new_script(std::uint32_t entry_ip) {
    if (idle_indices_.empty())
        return nullptr;
    const std::size_t index = idle_indices_.back();
    idle_indices_.pop_back();
    vcs::vm::ScriptThread& thread = thread_pool_[index];
    // Direct CTheScripts::StartNewScript reconstruction: acquire idle slot,
    // assign the pre-increment process id, Init its VM-visible state, set IP,
    // put it on active ownership and mark it active. The portable collection
    // models list membership by stable pool indices rather than guessed links.
    thread = vcs::vm::ScriptThread{};
    thread.scheduler_id = next_process_id_;
    thread.ip = entry_ip;
    thread.saved_ip = entry_ip;
    thread.locals.assign(kPortableLocalSlots, 0);
    thread.is_active = true;
    active_indices_.push_back(index);
    ++next_process_id_;
    return &thread;
}

vcs::vm::HandlerResult VcsMissionHost::handle_opcode_0289(vcs::vm::ScriptVm& vm,
                                                           vcs::vm::ScriptThread& parent) {
    vcs::vm::VmFault fault = vcs::vm::VmFault::None;
    const std::optional<std::int32_t> raw_selector = vm.read_parameter(parent, fault);
    if (!raw_selector)
        return vcs::vm::HandlerResult::Fault(fault);

    last_selector_ = *raw_selector;
    last_child_scheduler_id_.reset();
    last_child_result_.reset();
    std::int64_t selector = *raw_selector;
    // Exact native mode guard/remap, kept before any table access. The normal
    // bootstrap host uses mode zero; non-zero mode is exposed only so the
    // evidence-backed control path remains explicit rather than silently
    // removed.
    if (mission_mode_ > 0) {
        if (selector < 0xFFFE)
            return vcs::vm::HandlerResult::Continue();
        selector = 0xFFFF - selector;
    }
    const VcsMissionMetadata& metadata = container_.mission_metadata();
    if (selector < 0 || static_cast<std::uint64_t>(selector) + 1 >= metadata.offsets.size())
        return vcs::vm::HandlerResult::Fault(vcs::vm::VmFault::InvalidScriptTarget);

    const std::size_t mission_index = static_cast<std::size_t>(selector);
    const std::uint32_t mission_start = metadata.offsets[mission_index];
    const std::uint32_t mission_next = metadata.offsets[mission_index + 1];
    const std::uint32_t mission_size = mission_next > mission_start
        ? mission_next - mission_start
        : container_.largest_mission_script_size();
    const std::vector<std::uint8_t>& source = container_.source_body();
    if (mission_size == 0 || mission_start > source.size() ||
        mission_size > source.size() - mission_start ||
        mission_size > container_.largest_mission_script_size()) {
        return vcs::vm::HandlerResult::Fault(vcs::vm::VmFault::InvalidScriptTarget);
    }

    // The pair brackets only source positioning/copy and StartNewScript. The
    // recovered scope finalizer runs before the post-factory marker writes.
    enter_stream_scope();
    const std::vector<std::uint8_t> slice(
        source.begin() + static_cast<std::ptrdiff_t>(mission_start),
        source.begin() + static_cast<std::ptrdiff_t>(mission_start + mission_size));
    const bool copied = vm.script_space().write_bytes_at(container_.main_script_size(), slice);
    vcs::vm::ScriptThread* child = copied ? start_new_script(container_.main_script_size()) : nullptr;
    leave_stream_scope();
    if (!copied || child == nullptr)
        return vcs::vm::HandlerResult::Fault(vcs::vm::VmFault::InvalidScriptTarget);

    child->mission_launch_requested = true; // native +0x20A
    child->is_mission_script = true;        // native +0x217
    last_child_scheduler_id_ = child->scheduler_id;
    mission_active_ = true;
    const std::size_t globals_start = static_cast<std::size_t>(metadata.num_true_globals) * 4 + 8;
    const std::size_t globals_size = static_cast<std::size_t>(metadata.most_globals) * 4;
    if (globals_start > std::numeric_limits<std::uint32_t>::max() ||
        !vm.script_space().write_bytes_at(static_cast<std::uint32_t>(globals_start),
                                          std::vector<std::uint8_t>(globals_size, 0))) {
        return vcs::vm::HandlerResult::Fault(vcs::vm::VmFault::VariableStorageOutOfRange);
    }

    last_child_result_ = vm.process(*child, child_instruction_budget_);
    // Native 0289 returns Continue after its re-entrant child Process. A PC
    // bootstrap must nevertheless surface a child host boundary instead of
    // pretending the mission executed. Preserve its exact fault for the
    // outer runner; snapshot() retains the child's opcode/IP evidence.
    if (last_child_result_->fault != vcs::vm::VmFault::None)
        return vcs::vm::HandlerResult::Fault(last_child_result_->fault);
    return vcs::vm::HandlerResult::Continue();
}

std::optional<VcsScriptContainer> load_vcs_script_container(const IVcsFileSystem& fs,
                                                             const std::string& scm_path,
                                                             std::string& error) {
    std::vector<std::uint8_t> bytes;
    if (!fs.read_file(scm_path, bytes, error))
        return std::nullopt;
    return VcsScriptContainer::parse(std::move(bytes), error);
}

} // namespace vcs::host
