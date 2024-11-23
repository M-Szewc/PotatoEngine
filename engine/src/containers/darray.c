#include "containers/darray.h"

#include "core/pe_memory.h"
#include "core/logger.h"

void* _darray_create(u64 length, u64 stride) {
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = pe_allocate(header_size + array_size, MEMORY_TAG_DARRAY);
    pe_set_memory(new_array, 0, header_size + array_size);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_FIELD_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = 0;

    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void* array) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    pe_free(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void* array, u64 field) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void* array, u64 field, u64 value) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _darray_resize(void* array) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    void* temp = _darray_create(
        (DARRAY_RESIZE_FACTOR * darray_capacity(array)),
        stride);
    pe_copy_memory(temp, array, length * stride);

    _darray_field_set(temp, DARRAY_LENGTH, length);
    _darray_destroy(array);
    return temp;
}

void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    pe_copy_memory((void*)addr, value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);

    return array;
}

void _darray_pop(void* array, void* dst) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    pe_copy_memory(dst, (void*)addr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
}

void* _darray_pop_at(void* array, u64 index, void* dst) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (index >= length) {
        PE_ERROR("Index outside the bounds of this array! Length %i, index: %i, length, indeks");
        return array;
    }

    u64 addr = (u64)array;
    pe_copy_memory(dst, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward
    if (index != length - 1) {
        pe_copy_memory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;
}

void* _darray_insert_alt(void* array, u64 index, void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (index >= length) {
        PE_ERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;

    // If not on the last element, copy the rest onward.
    pe_copy_memory((void*)(addr + (index * stride)), value_ptr, stride);

    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}