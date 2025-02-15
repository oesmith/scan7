#ifndef _MBE_H_
#define _MBE_H_

#define MBE_PIN_CAN_CS (9)

#define MBE_ID_EASIMAP (0xcbe1101)
#define MBE_ID_ECU (0xcbe0111)

/** Maximum number of bytes that mbe_query(...) can fetch. */
#define MBE_MAX_QUERY_LENGTH 64

typedef enum {
  MBE_OK = 0,
  MBE_UNIMPLEMENTED = 1,
  MBE_INIT_TIMEOUT = 2,
  MBE_OUT_OF_BOUNDS = 3,
  MBE_SEND_ERROR = 4,
  MBE_RECV_TIMEOUT = 5,
  MBE_RECV_ERROR = 6,
  MBE_RECV_INVALID = 7,
  MBE_RECV_OUT_OF_SEQ = 8,
  MBE_RECV_BAD_HEADER = 9,
  MBE_VERSION_INVALID = 10,
  MBE_VERSION_OVERFLOW = 11,
  MBE_QUERY_TOO_MANY_BYTES = 12,
  MBE_QUERY_RESPONSE_INVALID = 13,
} mbe_error;

/**
 * Initialize the CANBUS interface.
 *
 * @return MBE_OK if successful.
 */
mbe_error mbe_init();

/**
 * Query the version of the MBE ECU.
 *
 * @param version Pointer to a buffer in which to store the version string.
 * @param len Length of the buffer.
 * @return MBE_OK if successful.
 */
mbe_error mbe_version(char* out_version, size_t len);

/**
 * Query data values from an MBE ECU.
 *
 * @param page Which page to fetch data from.
 * @param offsets A list of offsets within the page to fetch.
 * @param out_data Pointer to a buffer in which to store the fetched data.
 * @param len Number of data bytes to fetch (and hence also the size of the
 *    output buffer).
 * @return MBE_OK if successful.
 */
mbe_error mbe_query(
    uint8_t page,
    const uint8_t* offsets,
    uint8_t* out_data,
    size_t len);

// private?

#endif // _MBE_H_
