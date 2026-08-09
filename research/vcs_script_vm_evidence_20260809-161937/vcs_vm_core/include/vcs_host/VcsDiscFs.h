#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace vcs::host {

// Read-only file abstraction for a user-supplied, legally obtained VCS disc
// image or an extracted disc tree. Paths are ISO-style, slash-separated and
// case-insensitive. Implementations deliberately expose bytes only; no game
// asset is patched, unpacked, or persisted by this layer.
class IVcsFileSystem {
public:
    virtual ~IVcsFileSystem() = default;

    virtual bool read_file(const std::string& iso_path,
                           std::vector<std::uint8_t>& bytes,
                           std::string& error) const = 0;
    virtual std::string description() const = 0;
};

class DirectoryBackend final : public IVcsFileSystem {
public:
    explicit DirectoryBackend(std::string root_directory);

    bool read_file(const std::string& iso_path,
                   std::vector<std::uint8_t>& bytes,
                   std::string& error) const override;
    std::string description() const override;

private:
    std::string root_directory_;
};

// Minimal ISO9660 level-1 reader for VCS PS2 disc images. It reads the
// Primary Volume Descriptor and walks directory records on demand; it does
// not depend on a mounted drive or an external archive tool. Joliet/UDF and
// multi-extent files remain intentionally unsupported until an image needing
// them is evidenced.
class Ps2IsoBackend final : public IVcsFileSystem {
public:
    explicit Ps2IsoBackend(std::string iso_path);

    bool read_file(const std::string& iso_path,
                   std::vector<std::uint8_t>& bytes,
                   std::string& error) const override;
    std::string description() const override;

private:
    std::string iso_path_;
};

std::unique_ptr<IVcsFileSystem> open_vcs_disc_source(const std::string& path,
                                                      std::string& error);

} // namespace vcs::host
