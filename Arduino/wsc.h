
#ifndef WSC_H
#define WSC_H

#include <stddef.h> // size_t

#define WS_PROTO_HEADER_MAX_INPUT	12	// 8 bytes when len>65535 + 4 bytes for masking
#define WS_PROTO_HEADER_MAX_OUTPUT	8	// 8 bytes when len>65535
#define WS_MASK_SIZE			4

/// ws_handshake: feed it with crlf lines until it returns a !NULL line to answer back
/// @param  crlfline one full cr/lf line received from websocket client
/// @return a C string to send then to free, or NULL when still in the header reading process
const char* ws_handshake (const char* crlfline);

/// ws_decode: read websocket input flow, extract txt data
///            this is a wrapper which calls ws_decode_header() then ws_unmask()
/// @param  raw a pointer to a pointer to the raw input data
///         * *raw pointer may be modified to skip ws protocol header
///         * content may be modified (unmasked)
/// @param  raw_size size of raw input data
/// @return if (0),  come back later with more data
///                  and if (!*raw), there is a protocol decoding error
///         if (!0), valid user data length pointed by (modified) *raw
size_t ws_decode_full_frame (char** raw, size_t raw_size);

/// ws_decode_header
/// @param  raw see ws_decode()
/// @param  raw_size see ws_decode()
/// @param  real_data (returned) start of user data, still masked
///                              NULL on protocol error
/// @param  mask (returned) the ws mask to apply
/// @return if (0),  come back later with more data
///                  and if (!*raw), there is a protocol decoding error
///         if (!0), user data len this frame will contain (unrelated with raw_size)
size_t ws_decode_header (char* raw, size_t raw_size, char** real_data, char mask[WS_MASK_SIZE]);

/// ws_unmask
/// @param data user data to unmask
/// @param len length of data to unmask
/// @param offs offset of these data relative to user data start in the websocket frame
/// @param mask mask to apply
void ws_unmask (char* masked, size_t len, size_t offs, const char mask[4]);

/// ws_output_preamble_length: setup ws_out_preamble prior sending data
///
/// @param:  outsize is the user size to send
/// @return: ws_out_preamble's length to send prior to user data
size_t ws_output_preamble_length (size_t outsize);

//doc
extern char ws_out_preamble [WS_PROTO_HEADER_MAX_OUTPUT];

//doc
size_t ws_set_output_preamble (size_t outsize, char output_preamble[WS_PROTO_HEADER_MAX_OUTPUT]);

#endif // WSC_H
