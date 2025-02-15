#include <mcp_canbus.h>
#include "mbe.h"

#define MBE_INIT_RETRIES (10)
#define MBE_INIT_RETRY_DELAY_MS (100)

#define MBE_CAN_RATE (CAN_500KBPS)

#define MBE_ID_EASIMAP (0xcbe1101)
#define MBE_ID_ECU (0xcbe0111)
// 29-bit extended CAN ID mask.
#define MBE_ID_MASK (0x1fffffff)

#define MBE_MAX_MESSAGE_SIZE (4095)
// There's only 256 bytes in a page, so constrain queries to max 256 bytes.
#define MBE_MAX_QUERY_BYTES (256)

#define MBE_WAIT_STEP_MS (5)
#define MBE_WAIT_TIMEOUT_MS (200)

#define ISOTP_FRAME_SINGLE (0x0)
#define ISOTP_FRAME_FIRST (0x10)
#define ISOTP_FRAME_CONSECUTIVE (0x20)

MCP_CAN CAN(MBE_PIN_CAN_CS);

#define VER_REQ_LEN (3)
static const uint8_t VER_REQ[] = { 0x4, 0x0, 0xd };
#define VER_RES_PREFIX_LEN (3)
static const uint8_t VER_RES_PREFIX[] = { 0xe4, 0x0, 0xd };

#define DATA_REQ_PREFIX_LEN (5)
static const uint8_t DATA_REQ_PREFIX[] = { 0x01, 0x0, 0x0, 0x0, 0x0 };
#define DATA_RES_PREFIX_LEN (1)
static const uint8_t DATA_RES_PREFIX[] = { 0x81 };

mbe_error mbe_send(const uint8_t* msg, size_t len);
mbe_error mbe_recv();
mbe_error mbe_wait();
void mbe_flush();

static uint8_t mbe_data[MBE_MAX_MESSAGE_SIZE] = { 0 };
static size_t mbe_data_len = 0;

// *************************************************************************

mbe_error mbe_init() {
  int remaining = MBE_INIT_RETRIES;
  while (CAN_OK != CAN.begin(MBE_CAN_RATE)) {
    if (remaining-- == 0) {
      return MBE_INIT_TIMEOUT;
    }
    delay(MBE_INIT_RETRY_DELAY_MS);
  }

  CAN.init_Mask(0, 1, MBE_ID_ECU);
  CAN.init_Filt(0, 1, MBE_ID_MASK);

  return MBE_OK;
}

mbe_error mbe_version(char* const version, const size_t len) {
  mbe_flush();
  mbe_error err = mbe_send(VER_REQ, VER_REQ_LEN);
  if (err != MBE_OK) {
    return err;
  }
  err = mbe_recv();
  if (err != MBE_OK) {
    return err;
  }
  if (memcmp(mbe_data, VER_RES_PREFIX, VER_RES_PREFIX_LEN) != 0) {
    return MBE_VERSION_INVALID;
  }
  size_t ver_len = mbe_data_len - VER_RES_PREFIX_LEN;
  if (ver_len >= len) {
    return MBE_VERSION_OVERFLOW;
  }
  memcpy(version, &mbe_data[VER_RES_PREFIX_LEN], ver_len);
  version[ver_len] = '\0';
  return MBE_OK;
}

mbe_error mbe_query(
    const uint8_t page,
    const uint8_t* const offsets,
    uint8_t* const out_data,
    const size_t len) {
  static uint8_t req[DATA_REQ_PREFIX_LEN + 1 + MBE_MAX_QUERY_BYTES];
  if (len > MBE_MAX_QUERY_BYTES) {
    return MBE_QUERY_TOO_MANY_BYTES;
  }
  memcpy(req, DATA_REQ_PREFIX, DATA_REQ_PREFIX_LEN);
  req[DATA_REQ_PREFIX_LEN] = page;
  memcpy(&req[DATA_REQ_PREFIX_LEN + 1], offsets, len);
  mbe_error err = mbe_send(req, DATA_REQ_PREFIX_LEN + 1 + len);
  if (err != MBE_OK) {
    return err;
  }
  err = mbe_recv();
  if (err != MBE_OK) {
    return err;
  }
  if (memcmp(mbe_data, DATA_RES_PREFIX, DATA_RES_PREFIX_LEN) != 0) {
    return MBE_QUERY_RESPONSE_INVALID;
  }
  if (mbe_data_len != DATA_RES_PREFIX_LEN + len) {
    return MBE_QUERY_RESPONSE_INVALID;
  }
  memcpy(out_data, &mbe_data[DATA_RES_PREFIX_LEN], len);
  return MBE_OK;
}

// *************************************************************************

mbe_error mbe_send(const uint8_t* msg, const size_t len) {
  if (len == 0 || len > 4095) {
    return MBE_OUT_OF_BOUNDS;
  }
  mbe_flush();
  uint8_t frame[8] = {0};
  if (len <= 7) {
    // Send in a single frame.
    frame[0] = (uint8_t) len;
    memcpy(&frame[1], msg, len);
    if (CAN.sendMsgBuf(MBE_ID_EASIMAP, 1, 8, frame) != CAN_OK) {
      return MBE_SEND_ERROR;
    }
    return MBE_OK;
  }
  // Multiple frames.
  frame[0] = 0x10 | (uint8_t)(len >> 8);
  frame[1] = (uint8_t)(len & 0xff);
  memcpy(&frame[2], msg, 6);
  CAN.sendMsgBuf(MBE_ID_EASIMAP, 1, 8, frame);
  size_t offset = 6;
  uint8_t idx = 1;
  while (offset < len) {
    size_t n = min(7, len - offset);
    frame[0] = 0x20 | (uint8_t)(idx & 0xf);
    memcpy(&frame[1], &msg[offset], n);
    if (CAN.sendMsgBuf(MBE_ID_EASIMAP, 1, 8, frame) != CAN_OK) {
      return MBE_SEND_ERROR;
    }
    idx++;
    offset += n;
  }
  return MBE_OK;
}

// Waits until a packet is available to read.
mbe_error mbe_wait() {
  uint16_t ms = 0;
  for (uint16_t ms = 0; ms < MBE_WAIT_TIMEOUT_MS; ms += MBE_WAIT_STEP_MS) {
    if (CAN.checkReceive() == CAN_MSGAVAIL) {
      return MBE_OK;
    }
    delay(MBE_WAIT_STEP_MS);
  }
  return MBE_RECV_TIMEOUT;
}

mbe_error mbe_recv() {
  // Read the first packet.
  mbe_error err = mbe_wait();
  if (err != MBE_OK) {
    return err;
  }
  uint8_t len;
  uint8_t buf[8];
  if (CAN.readMsgBuf(&len, buf) != CAN_OK) {
    return MBE_RECV_ERROR;
  }
  if (len < 2) {
    return MBE_RECV_INVALID;
  }
  uint8_t type = buf[0] & 0xf0;
  if (type == ISOTP_FRAME_SINGLE) {
    mbe_data_len = buf[0] & 0x7;
    memcpy(mbe_data, &buf[1], mbe_data_len);
  } else if (type != ISOTP_FRAME_FIRST) {
    return MBE_RECV_BAD_HEADER;
  }
  size_t total = (((size_t)buf[0] & 0xf) << 8) + buf[1];
  size_t received = len - 2;
  memcpy(mbe_data, &buf[2], received);

  uint8_t seq = 1;
  while (received < total) {
    err = mbe_wait();
    if (err != MBE_OK) {
      return err;
    }
    if (CAN.readMsgBuf(&len, buf) != CAN_OK) {
      return MBE_RECV_BAD_HEADER;
    }
    if ((buf[0] & 0xf0) != ISOTP_FRAME_CONSECUTIVE) {
      return MBE_RECV_BAD_HEADER;
    }
    if ((buf[0] & 0xf) != seq) {
      return MBE_RECV_OUT_OF_SEQ;
    }
    size_t data_len = min(len - 1, total - received);
    memcpy(&mbe_data[received], &buf[1], data_len);
    received += data_len;
    seq = (seq + 1) % 16;
  }
  mbe_data_len = received;
  return MBE_OK;
}

void mbe_flush() {
  while (CAN.checkReceive() == CAN_MSGAVAIL) {
    uint8_t len;
    uint8_t buf[8];
    CAN.readMsgBuf(&len, buf);
    // Delay just in case there's more messages queued.
    delay(10);
  }
}
