#include "vcs_host/VcsDiscFs.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>

namespace vcs::host {
namespace {

constexpr std::uint32_t kIsoSectorSize = 2048;
constexpr std::uint32_t kPrimaryVolumeDescriptorSector = 16;

struct IsoRecord {
    std::uint32_t extent_lba = 0;
    std::uint32_t data_length = 0;
    bool directory = false;
};

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

std::string upper_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

bool normalise_path(const std::string& input, std::vector<std::string>& components,
                    std::string& error) {
    components.clear();
    std::string component;
    for (const char raw : input) {
        const char c = raw == '\\' ? '/' : raw;
        if (c == '/') {
            if (component.empty())
                continue;
            if (component == "." || component == "..") {
                error = "disc path must not contain . or ..";
                return false;
            }
            components.push_back(upper_ascii(component));
            component.clear();
        } else {
            component.push_back(c);
        }
    }
    if (!component.empty()) {
        if (component == "." || component == "..") {
            error = "disc path must not contain . or ..";
            return false;
        }
        components.push_back(upper_ascii(component));
    }
    if (components.empty()) {
        error = "disc path is empty";
        return false;
    }
    return true;
}

bool read_file_exact(const std::filesystem::path& path, std::uint64_t offset,
                     std::uint64_t length, std::vector<std::uint8_t>& bytes,
                     std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = "cannot open read-only source: " + path.string();
        return false;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff stream_size = input.tellg();
    if (stream_size < 0 || offset > static_cast<std::uint64_t>(stream_size) ||
        length > static_cast<std::uint64_t>(stream_size) - offset ||
        length > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        error = "requested range exceeds read-only source";
        return false;
    }
    input.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    bytes.resize(static_cast<std::size_t>(length));
    if (length != 0 && !input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(length))) {
        error = "short read from read-only source";
        return false;
    }
    return true;
}

bool read_iso_range(const std::filesystem::path& iso_path, std::uint64_t offset,
                    std::uint64_t length, std::vector<std::uint8_t>& bytes,
                    std::string& error) {
    return read_file_exact(iso_path, offset, length, bytes, error);
}

bool read_iso_record(const std::vector<std::uint8_t>& bytes, std::size_t offset,
                     IsoRecord& record, std::string& identifier, std::size_t& record_length,
                     std::string& error) {
    if (offset >= bytes.size()) {
        error = "ISO directory record offset exceeds buffer";
        return false;
    }
    record_length = bytes[offset];
    if (record_length == 0)
        return true;
    if (record_length < 34 || record_length > bytes.size() - offset) {
        error = "malformed ISO9660 directory record";
        return false;
    }
    const std::size_t id_length = bytes[offset + 32];
    if (33 + id_length > record_length) {
        error = "ISO9660 directory identifier exceeds record";
        return false;
    }
    if (!read_u32_le(bytes, offset + 2, record.extent_lba) ||
        !read_u32_le(bytes, offset + 10, record.data_length)) {
        error = "truncated ISO9660 extent record";
        return false;
    }
    record.directory = (bytes[offset + 25] & 0x02U) != 0;
    identifier.assign(reinterpret_cast<const char*>(bytes.data() + offset + 33), id_length);
    const std::size_t version = identifier.find(';');
    if (version != std::string::npos)
        identifier.resize(version);
    identifier = upper_ascii(identifier);
    return true;
}

bool get_iso_root(const std::filesystem::path& iso_path, IsoRecord& root, std::string& error) {
    std::vector<std::uint8_t> descriptor;
    if (!read_iso_range(iso_path, static_cast<std::uint64_t>(kPrimaryVolumeDescriptorSector) * kIsoSectorSize,
                        kIsoSectorSize, descriptor, error))
        return false;
    if (descriptor.size() != kIsoSectorSize || descriptor[0] != 1 ||
        std::string(reinterpret_cast<const char*>(descriptor.data() + 1), 5) != "CD001" ||
        descriptor[6] != 1) {
        error = "no ISO9660 primary volume descriptor at sector 16";
        return false;
    }
    std::string ignored;
    std::size_t record_length = 0;
    return read_iso_record(descriptor, 156, root, ignored, record_length, error) && record_length != 0 && root.directory;
}

bool find_iso_child(const std::filesystem::path& iso_path, const IsoRecord& directory,
                    const std::string& requested_name, IsoRecord& child, std::string& error) {
    const std::uint64_t directory_offset = static_cast<std::uint64_t>(directory.extent_lba) * kIsoSectorSize;
    std::vector<std::uint8_t> bytes;
    if (!read_iso_range(iso_path, directory_offset, directory.data_length, bytes, error))
        return false;

    std::size_t offset = 0;
    while (offset < bytes.size()) {
        const std::size_t sector_remaining = kIsoSectorSize - (offset % kIsoSectorSize);
        if (bytes[offset] == 0) {
            offset += sector_remaining;
            continue;
        }
        IsoRecord record;
        std::string identifier;
        std::size_t record_length = 0;
        if (!read_iso_record(bytes, offset, record, identifier, record_length, error))
            return false;
        if (record_length == 0) {
            offset += sector_remaining;
            continue;
        }
        if (identifier == requested_name) {
            child = record;
            return true;
        }
        offset += record_length;
    }
    error = "ISO9660 path component not found: " + requested_name;
    return false;
}

} // namespace

DirectoryBackend::DirectoryBackend(std::string root_directory)
    : root_directory_(std::move(root_directory)) {}

bool DirectoryBackend::read_file(const std::string& iso_path,
                                 std::vector<std::uint8_t>& bytes,
                                 std::string& error) const {
    std::vector<std::string> components;
    if (!normalise_path(iso_path, components, error))
        return false;
    std::filesystem::path target(root_directory_);
    for (const std::string& component : components)
        target /= component;
    std::error_code ec;
    if (!std::filesystem::is_regular_file(target, ec)) {
        error = "file not found in extracted disc tree: " + target.string();
        return false;
    }
    const auto length = std::filesystem::file_size(target, ec);
    if (ec) {
        error = "cannot determine file size: " + target.string();
        return false;
    }
    return read_file_exact(target, 0, length, bytes, error);
}

std::string DirectoryBackend::description() const {
    return "directory:" + root_directory_;
}

Ps2IsoBackend::Ps2IsoBackend(std::string iso_path)
    : iso_path_(std::move(iso_path)) {}

bool Ps2IsoBackend::read_file(const std::string& iso_path,
                              std::vector<std::uint8_t>& bytes,
                              std::string& error) const {
    std::vector<std::string> components;
    if (!normalise_path(iso_path, components, error))
        return false;

    const std::filesystem::path image(iso_path_);
    std::error_code ec;
    if (!std::filesystem::is_regular_file(image, ec)) {
        error = "ISO image does not exist: " + image.string();
        return false;
    }

    IsoRecord current;
    if (!get_iso_root(image, current, error))
        return false;
    for (std::size_t index = 0; index < components.size(); ++index) {
        IsoRecord next;
        if (!find_iso_child(image, current, components[index], next, error))
            return false;
        if (index + 1 < components.size() && !next.directory) {
            error = "ISO9660 path component is not a directory: " + components[index];
            return false;
        }
        current = next;
    }
    if (current.directory) {
        error = "ISO9660 path names a directory, not a file";
        return false;
    }
    return read_iso_range(image, static_cast<std::uint64_t>(current.extent_lba) * kIsoSectorSize,
                          current.data_length, bytes, error);
}

std::string Ps2IsoBackend::description() const {
    return "iso9660:" + iso_path_;
}

std::unique_ptr<IVcsFileSystem> open_vcs_disc_source(const std::string& path,
                                                      std::string& error) {
    std::error_code ec;
    const std::filesystem::path source(path);
    if (std::filesystem::is_directory(source, ec))
        return std::make_unique<DirectoryBackend>(path);
    if (std::filesystem::is_regular_file(source, ec))
        return std::make_unique<Ps2IsoBackend>(path);
    error = "input is neither an extracted-disc directory nor an ISO image: " + path;
    return nullptr;
}

} // namespace vcs::host
