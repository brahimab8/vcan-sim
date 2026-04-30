#include <gtest/gtest.h>
#include "signal_encoder.h"
#include "can_frame.h"

// ---------------------------------------------------------------------------
// Helper: create a zeroed CanFrame for each test
// ---------------------------------------------------------------------------
static CanFrame makeEmptyFrame()
{
    CanFrame frame{};
    frame.id  = 0;
    frame.dlc = 0;
    frame.data.fill(0);
    return frame;
}

// ---------------------------------------------------------------------------
// encodeUint16LE
// ---------------------------------------------------------------------------

TEST(SignalEncoderUint16LE, EncodesLSBAtLowerOffset)
{
    CanFrame frame = makeEmptyFrame();
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, 0x1770));

    EXPECT_EQ(frame.data[0], 0x70);  // LSB
    EXPECT_EQ(frame.data[1], 0x17);  // MSB
}

TEST(SignalEncoderUint16LE, EncodesAtNonZeroOffset)
{
    CanFrame frame = makeEmptyFrame();
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 2, 0x1770));

    EXPECT_EQ(frame.data[2], 0x70);
    EXPECT_EQ(frame.data[3], 0x17);
    EXPECT_EQ(frame.data[0], 0x00);  // bytes before offset untouched
    EXPECT_EQ(frame.data[1], 0x00);
}

TEST(SignalEncoderUint16LE, EncodesZero)
{
    CanFrame frame = makeEmptyFrame();
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, 0x0000));

    EXPECT_EQ(frame.data[0], 0x00);
    EXPECT_EQ(frame.data[1], 0x00);
}

TEST(SignalEncoderUint16LE, EncodesMaxValue)
{
    CanFrame frame = makeEmptyFrame();
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, 0xFFFF));

    EXPECT_EQ(frame.data[0], 0xFF);
    EXPECT_EQ(frame.data[1], 0xFF);
}

// ---------------------------------------------------------------------------
// decodeUint16LE
// ---------------------------------------------------------------------------

TEST(SignalEncoderUint16LE, DecodeRoundTrip)
{
    CanFrame frame = makeEmptyFrame();
    const uint16_t original = 6000;
    uint16_t decoded = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, original));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 0, decoded));
    EXPECT_EQ(decoded, original);
}

TEST(SignalEncoderUint16LE, DecodeAtNonZeroOffset)
{
    CanFrame frame = makeEmptyFrame();
    uint16_t decoded = 0;
    uint16_t untouched = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 4, 0xABCD));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 4, decoded));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 0, untouched)); // other bytes untouched
    EXPECT_EQ(decoded, 0xABCD);
    EXPECT_EQ(untouched, 0x0000);
}

TEST(SignalEncoderUint16LE, DecodeZero)
{
    CanFrame frame = makeEmptyFrame();
    uint16_t decoded = 0xFFFF;

    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, 0x0000));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 0, decoded));
    EXPECT_EQ(decoded, 0x0000);
}

TEST(SignalEncoderUint16LE, DecodeMaxValue)
{
    CanFrame frame = makeEmptyFrame();
    uint16_t decoded = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, 0xFFFF));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 0, decoded));
    EXPECT_EQ(decoded, 0xFFFF);
}

// ---------------------------------------------------------------------------
// encodeUint8 / decodeUint8
// ---------------------------------------------------------------------------

TEST(SignalEncoderUint8, EncodesCorrectByte)
{
    CanFrame frame = makeEmptyFrame();
    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 2, 0x7D));

    EXPECT_EQ(frame.data[2], 0x7D);
    EXPECT_EQ(frame.data[0], 0x00);  // other bytes untouched
    EXPECT_EQ(frame.data[1], 0x00);
}

TEST(SignalEncoderUint8, DecodeRoundTrip)
{
    CanFrame frame = makeEmptyFrame();
    const uint8_t original = 125;
    uint8_t decoded = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 2, original));
    EXPECT_TRUE(SignalEncoder::decodeUint8(frame, 2, decoded));
    EXPECT_EQ(decoded, original);
}

TEST(SignalEncoderUint8, DecodeZero)
{
    CanFrame frame = makeEmptyFrame();
    uint8_t decoded = 0xFF;

    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 0, 0x00));
    EXPECT_TRUE(SignalEncoder::decodeUint8(frame, 0, decoded));
    EXPECT_EQ(decoded, 0x00);
}

TEST(SignalEncoderUint8, DecodeMaxValue)
{
    CanFrame frame = makeEmptyFrame();
    uint8_t decoded = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 0, 0xFF));
    EXPECT_TRUE(SignalEncoder::decodeUint8(frame, 0, decoded));
    EXPECT_EQ(decoded, 0xFF);
}

// ---------------------------------------------------------------------------
// Byte isolation: Encoding one value does not corrupt adjacent bytes
// ---------------------------------------------------------------------------

TEST(SignalEncoderIsolation, MultipleUint16LEDoNotOverlap)
{
    CanFrame frame = makeEmptyFrame();
    uint16_t d0 = 0;
    uint16_t d2 = 0;
    uint16_t d4 = 0;
    uint16_t d6 = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 0, 0x1111));
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 2, 0x2222));
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 4, 0x3333));
    EXPECT_TRUE(SignalEncoder::encodeUint16LE(frame, 6, 0x4444));

    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 0, d0));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 2, d2));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 4, d4));
    EXPECT_TRUE(SignalEncoder::decodeUint16LE(frame, 6, d6));
    EXPECT_EQ(d0, 0x1111);
    EXPECT_EQ(d2, 0x2222);
    EXPECT_EQ(d4, 0x3333);
    EXPECT_EQ(d6, 0x4444);
}

TEST(SignalEncoderIsolation, Uint8DoesNotCorruptNeighbours)
{
    CanFrame frame = makeEmptyFrame();
    uint8_t d0 = 0;
    uint8_t d1 = 0;
    uint8_t d2 = 0;

    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 0, 0xAA));
    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 1, 0xBB));
    EXPECT_TRUE(SignalEncoder::encodeUint8(frame, 2, 0xCC));

    EXPECT_TRUE(SignalEncoder::decodeUint8(frame, 0, d0));
    EXPECT_TRUE(SignalEncoder::decodeUint8(frame, 1, d1));
    EXPECT_TRUE(SignalEncoder::decodeUint8(frame, 2, d2));
    EXPECT_EQ(d0, 0xAA);
    EXPECT_EQ(d1, 0xBB);
    EXPECT_EQ(d2, 0xCC);
}

// ---------------------------------------------------------------------------
// Checked APIs and bounds behavior
// ---------------------------------------------------------------------------

TEST(SignalEncoderBounds, EncodeUint16LERejectsOutOfBoundsOffset)
{
    CanFrame frame = makeEmptyFrame();

    EXPECT_FALSE(SignalEncoder::encodeUint16LE(frame, 7, 0x1234));
    EXPECT_EQ(frame.data[7], 0x00);
}

TEST(SignalEncoderBounds, EncodeUint8RejectsOutOfBoundsOffset)
{
    CanFrame frame = makeEmptyFrame();

    EXPECT_FALSE(SignalEncoder::encodeUint8(frame, 8, 0xAB));
    EXPECT_EQ(frame.data[7], 0x00);  // frame untouched
}

TEST(SignalEncoderBounds, DecodeUint16LERejectsOutOfBoundsOffset)
{
    CanFrame frame = makeEmptyFrame();
    uint16_t value = 0xFFFF;

    EXPECT_FALSE(SignalEncoder::decodeUint16LE(frame, 7, value));
    EXPECT_EQ(value, 0x0000);
}

TEST(SignalEncoderBounds, DecodeUint8RejectsOutOfBoundsOffset)
{
    CanFrame frame = makeEmptyFrame();
    uint8_t value = 0xFF;

    EXPECT_FALSE(SignalEncoder::decodeUint8(frame, 8, value));
    EXPECT_EQ(value, 0x00);
}