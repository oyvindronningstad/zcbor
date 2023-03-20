/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_print.h>


void zcbor_print_compare_lines(const uint8_t *str1, const uint8_t *str2, size_t size)
{
	for (size_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str1[j]);
	}
	zcbor_do_print("\r\n");
	for (size_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str2[j]);
	}
	zcbor_do_print("\r\n");
	for (size_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str1[j] != str2[j]);
	}
	zcbor_do_print("\r\n");
	zcbor_do_print("\r\n");
}


void zcbor_print_compare_strings(const uint8_t *str1, const uint8_t *str2, size_t size)
{
	const size_t col_width = 16;

	for (size_t i = 0; i <= size / col_width; i++) {
		zcbor_do_print("line %zu (char %zu)\r\n", i, i*col_width);
		zcbor_print_compare_lines(&str1[i*col_width], &str2[i*col_width],
			MIN(col_width, (size - i*col_width)));
	}
	zcbor_do_print("\r\n");
}


void zcbor_print_compare_strings_diff(const uint8_t *str1, const uint8_t *str2, size_t size)
{
	const size_t col_width = 16;
	bool printed = false;

	for (size_t i = 0; i <= size / col_width; i++) {
		if (memcmp(&str1[i*col_width], &str2[i*col_width], MIN(col_width, (size - i*col_width))) != 0) {
			zcbor_do_print("line %zu (char %zu)\r\n", i, i*col_width);
			zcbor_print_compare_lines(&str1[i*col_width], &str2[i*col_width],
				MIN(col_width, (size - i*col_width)));
			printed = true;
		}
	}
	if (printed) {
		zcbor_do_print("\r\n");
	}
}


const char *zcbor_error_str(int error)
{
	#define ZCBOR_ERR_CASE(err) case err: \
		return #err; /* The literal is static per C99 6.4.5 paragraph 5. */\

	switch(error) {
		ZCBOR_ERR_CASE(ZCBOR_SUCCESS)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NO_BACKUP_MEM)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NO_BACKUP_ACTIVE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_LOW_ELEM_COUNT)
		ZCBOR_ERR_CASE(ZCBOR_ERR_HIGH_ELEM_COUNT)
		ZCBOR_ERR_CASE(ZCBOR_ERR_INT_SIZE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_FLOAT_SIZE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ADDITIONAL_INVAL)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NO_PAYLOAD)
		ZCBOR_ERR_CASE(ZCBOR_ERR_PAYLOAD_NOT_CONSUMED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_WRONG_TYPE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_WRONG_VALUE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_WRONG_RANGE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ITERATIONS)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ASSERTION)
		ZCBOR_ERR_CASE(ZCBOR_ERR_PAYLOAD_OUTDATED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ELEM_NOT_FOUND)
		ZCBOR_ERR_CASE(ZCBOR_ERR_MAP_MISALIGNED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ELEMS_NOT_PROCESSED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NOT_AT_END)
		ZCBOR_ERR_CASE(ZCBOR_ERR_MAP_FLAGS_NOT_AVAILABLE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_INVALID_VALUE_ENCODING)
	}
	#undef ZCBOR_ERR_CASE

	return "ZCBOR_ERR_UNKNOWN";
}


void zcbor_print_error(int error)
{
	zcbor_do_print("%s\r\n", zcbor_error_str(error));
}

#ifdef ZCBOR_PRINT_CBOR
static bool indent_printed = false;


void zcbor_print_indent(size_t indent_len)
{
	if (!indent_printed) {
		for (int i = 0; i < indent_len; i++) {
			zcbor_do_print("| ");
		}
		indent_printed = true;
	}
}


void zcbor_print_newline(void)
{
	zcbor_do_print("\r\n");
	indent_printed = false;
}

#define BYTES_PER_LINE 16


void zcbor_print_str(const uint8_t *str, size_t len, size_t indent_len)
{
	for (size_t i = 0; i < len; i++) {
		if (!(i % BYTES_PER_LINE)) {
			if (i > 0) {zcbor_print_newline();}
			zcbor_print_indent(indent_len);
			zcbor_do_print("0x");
		}
		zcbor_do_print("%02x ", str[i]);
	}
}

static void print_cbor(struct zcbor_element *elem, size_t indent_len);

void zcbor_print_bstr_payload(zcbor_state_t *state, size_t len, size_t indent_len)
{
	struct zcbor_element elem;

	if (len == 0) {
		return;
	}
	zcbor_print_str(state->payload, len, indent_len);
	zcbor_print_newline();

	if (zcbor_any_decode(state, &elem)
			&& (state->payload == state->payload_end)) {
		(void)print_cbor(&elem, indent_len);
	}
}

#ifdef ZCBOR_PRINT_CBOR_PRETTY
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_common.h"
#define RESET_COLOR   "\x1B[0m"

#ifndef ZCBOR_PRINT_CBOR_COLOR_HEADER
#define ZCBOR_PRINT_CBOR_COLOR_HEADER "\x1B[31m" /* red */
#endif
#ifndef ZCBOR_PRINT_CBOR_COLOR_VALUE
#define ZCBOR_PRINT_CBOR_COLOR_VALUE "\x1B[34m" /* blue */
#endif
#ifndef ZCBOR_PRINT_CBOR_COLOR_DESC
#define ZCBOR_PRINT_CBOR_COLOR_DESC "\x1B[32m" /* green */
#endif
#ifndef ZCBOR_PRINT_CBOR_COLOR_TAG
#define ZCBOR_PRINT_CBOR_COLOR_TAG "\x1B[33m" /* yellow */
#endif


void zcbor_print_tag(uint32_t tag, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_TAG "0x%02x ", tag);
}


static void print_uint(uint64_t value)
{
	zcbor_do_print("%" PRIu64, value);
}


static void print_nint(int64_t value)
{
	zcbor_do_print("%" PRId64, value);
}


void zcbor_print_simple(struct zcbor_element *elem)
{
	const char *simple_strings[] = {"false", "true", "nil", "undefined"};


	switch(elem->special) {
	case ZCBOR_SPECIAL_VAL_FALSE:
	case ZCBOR_SPECIAL_VAL_TRUE:
	case ZCBOR_SPECIAL_VAL_UNDEF:
	case ZCBOR_SPECIAL_VAL_NIL:
		zcbor_do_print("%s", simple_strings[elem->special - ZCBOR_SPECIAL_VAL_FALSE]);
		break;
	case ZCBOR_SPECIAL_VAL_SIMPLE:
		zcbor_do_print("simple<%u>", (uint8_t)elem->value);
		break;
	case ZCBOR_SPECIAL_VAL_FLOAT16:
		zcbor_do_print("%f", zcbor_float16_to_32(elem->float16));
		break;
	case ZCBOR_SPECIAL_VAL_FLOAT32:
		zcbor_do_print("%f", elem->float32);
		break;
	case ZCBOR_SPECIAL_VAL_FLOAT64:
		zcbor_do_print("%f", elem->float64);
		break;
	}
}


static const char *header_byte_strings[] = {
	"", "", "bstr", "tstr", "list", "map", "", ""
};


void zcbor_print_value(struct zcbor_element *elem, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_HEADER "0x%02x " ZCBOR_PRINT_CBOR_COLOR_VALUE,
			*elem->encoded_value);

	size_t len = elem->encoded_payload - elem->encoded_value;
	if (len > 0) {
		zcbor_print_str(elem->encoded_value + 1, len - 1, 0);
	}
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_DESC "(");

	switch(elem->type) {
	case ZCBOR_MAJOR_TYPE_PINT:
		print_uint(elem->value);
		break;
	case ZCBOR_MAJOR_TYPE_NINT:
		print_nint(elem->neg_value);
		break;
	case ZCBOR_MAJOR_TYPE_BSTR:
	case ZCBOR_MAJOR_TYPE_TSTR:
	case ZCBOR_MAJOR_TYPE_LIST:
	case ZCBOR_MAJOR_TYPE_MAP:
		if (elem->additional == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
			zcbor_do_print("%s", header_byte_strings[elem->type]);
		} else {
			zcbor_do_print("%s<%" PRIu64 ">", header_byte_strings[elem->type], elem->value);
		}
		break;
	case ZCBOR_MAJOR_TYPE_SIMPLE:
		zcbor_print_simple(elem);
		break;
	default:
		/* ignore */
	}

	zcbor_do_print(")" RESET_COLOR);
	zcbor_print_newline();
}


void zcbor_print_tstr_payload(zcbor_state_t *state, size_t len, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print("\"");
	size_t prev_i = 0;

	for (size_t i = 0; i < len; i++) {
		if (state->payload[i] == '\n') {
			/* Add indent after newlines. */
			zcbor_do_print("%.*s", (int)(i - prev_i), state->payload + prev_i);
			prev_i = i + 1;
			zcbor_print_newline();
			zcbor_print_indent(indent_len);
		}
	}
	zcbor_do_print("%.*s\"", (int)(len - prev_i), state->payload + prev_i);
	zcbor_print_newline();
}


void zcbor_print_end(zcbor_major_type_t major_type, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_HEADER "0xff " ZCBOR_PRINT_CBOR_COLOR_DESC "(%s end)" RESET_COLOR,
		header_byte_strings[major_type]);
	zcbor_print_newline();
}

#else


void zcbor_print_tstr_payload(zcbor_state_t *state, size_t len, size_t indent_len)
{
	zcbor_do_print("\"%.*s\"", (int)len, state->payload);
	zcbor_print_newline();
}


void zcbor_print_value(struct zcbor_element *elem, size_t indent_len)
{
	size_t len = elem->encoded_payload - elem->encoded_value;

	zcbor_print_str(elem->encoded_value, len, indent_len);

	if (len) {
		if (elem->additional == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
			zcbor_do_print("(start)");
		} else if (elem->type == ZCBOR_MAJOR_TYPE_NINT) {
			zcbor_do_print("(%" PRIi64 ")", elem->neg_value);
		// } else if (elem->type ==)
		} else {
			zcbor_do_print("(%" PRIu64 ")", elem->value);
		}
		zcbor_print_newline();
	}
}


void zcbor_print_tag(uint32_t tag, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print("0x%02" PRIx32 " ", tag);
}


void zcbor_print_end(zcbor_major_type_t major_type, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print("0xff (end)");
	zcbor_print_newline();
}

#endif


static void print_cbor(struct zcbor_element *elem, size_t indent_len)
{
	zcbor_state_t state[2];
	uint32_t tag;

	zcbor_new_state_from_string(state, 2, &elem->encoded);
	while (zcbor_tag_decode(state, &tag)) {
		zcbor_print_tag(tag, indent_len);
	}

	zcbor_print_value(elem, indent_len);
	state->payload = elem->encoded_payload;

	switch(elem->type) {
	case ZCBOR_MAJOR_TYPE_BSTR:
		zcbor_print_bstr_payload(state, elem->value, indent_len + 1);
		break;
	case ZCBOR_MAJOR_TYPE_TSTR:
		zcbor_print_tstr_payload(state, elem->value, indent_len + 1);
		break;
	case ZCBOR_MAJOR_TYPE_LIST:
	case ZCBOR_MAJOR_TYPE_MAP:
		state->elem_count = elem->value;
		if (elem->type == ZCBOR_MAJOR_TYPE_MAP) {
			state->elem_count *= 2;
		}
		if (elem->additional == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
			state->elem_count = ZCBOR_LARGE_ELEM_COUNT;
			state->decode_state.indefinite_length_array = true;
		}
		while (state->elem_count > 0) {
			struct zcbor_element child;
			bool res = zcbor_any_decode(state, &child);
			if (res) {
				print_cbor(&child, indent_len + 1);
			} else {
				if(!zcbor_array_at_end(state)) {
					zcbor_do_print("Could not print (%s)\n", zcbor_error_str(zcbor_peek_error(state)));
				} else {
					zcbor_do_print("End of array.\n");
				}
				break;
			}
		}
		if (elem->additional == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
			zcbor_print_end(elem->type, indent_len);
		}
		break;
	default:
		/* do nothing */
	}
}


void zcbor_print_cbor(struct zcbor_element *elem)
{
	print_cbor(elem, 0);
}

#endif
