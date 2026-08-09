#ifndef PS2_RENDER_BOUNDARY_H
#define PS2_RENDER_BOUNDARY_H

#include <cstdint>
#include <functional>

// This is an observation seam, deliberately not a renderer replacement.
// A caller may use it to fingerprint VIF/VU/GS traffic and recover an HLE
// boundary above it.  Returning from an observer never suppresses the guest
// VIF, VU or GS path.
enum class PS2RenderBoundaryKind : uint8_t
{
    Vif1Mscal,
    Vif1Mscnt,
    GifPacket,
};

struct PS2RenderBoundaryEvent
{
    PS2RenderBoundaryKind kind{};

    // VIF1 MSCAL/MSCNT state. startPc is zero for MSCNT, which resumes the
    // previously running VU1 microprogram.
    uint32_t vu1StartPc = 0;
    uint32_t vifTop = 0;
    uint32_t vifItop = 0;
    uint64_t vu1CodeGeneration = 0;
    uint64_t vu1CodeFingerprint = 0;

    // GIF state. gifPath uses the numeric PS2 path id (1, 2 or 3); it is zero
    // for VIF events. The fingerprint is FNV-1a over the packet bytes and is
    // intended for grouping equivalent draws without retaining game data.
    uint8_t gifPath = 0;
    bool path2DirectHl = false;
    uint32_t packetSizeBytes = 0;
    uint64_t packetFingerprint = 0;
};

using PS2RenderBoundaryObserver = std::function<void(const PS2RenderBoundaryEvent &)>;

inline uint64_t ps2RenderBoundaryFingerprint(const uint8_t *data, uint32_t sizeBytes)
{
    constexpr uint64_t kOffsetBasis = 14695981039346656037ull;
    constexpr uint64_t kPrime = 1099511628211ull;
    uint64_t value = kOffsetBasis;
    if (!data)
    {
        return value;
    }
    for (uint32_t index = 0; index < sizeBytes; ++index)
    {
        value ^= data[index];
        value *= kPrime;
    }
    return value;
}

#endif // PS2_RENDER_BOUNDARY_H
