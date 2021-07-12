/*
 * The index entry vector functions
 *
 * Copyright (C) 2010-2021, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <types.h>

#include "libfsntfs_data_run.h"
#include "libfsntfs_definitions.h"
#include "libfsntfs_index_entry.h"
#include "libfsntfs_index_entry_vector.h"
#include "libfsntfs_io_handle.h"
#include "libfsntfs_libcdata.h"
#include "libfsntfs_libcerror.h"
#include "libfsntfs_libfdata.h"
#include "libfsntfs_mft_attribute.h"
#include "libfsntfs_unused.h"

/* Creates an index entry vector
 * Make sure the value index_entry_vector is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_index_entry_vector_initialize(
     libfdata_vector_t **index_entry_vector,
     libfsntfs_io_handle_t *io_handle,
     libfsntfs_mft_attribute_t *mft_attribute,
     libcerror_error_t **error )
{
	libfdata_vector_t *safe_index_entry_vector   = NULL;
	libfsntfs_data_run_t *data_run               = NULL;
	static char *function                        = "libfsntfs_index_entry_vector_initialize";
	size64_t attribute_data_vcn_size             = 0;
	size64_t calculated_allocated_data_size      = 0;
	size64_t stored_allocated_data_size          = 0;
	off64_t attribute_data_vcn_offset            = 0;
	off64_t calculated_attribute_data_vcn_offset = 0;
	uint16_t attribute_data_flags                = 0;
	int attribute_index                          = 0;
	int entry_index                              = 0;
	int number_of_entries                        = 0;
	int segment_index                            = 0;

	if( index_entry_vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid index entry vector.",
		 function );

		return( -1 );
	}
	if( *index_entry_vector != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid index entry vector value already set.",
		 function );

		return( -1 );
	}
	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( io_handle->cluster_block_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid IO handle - cluster block size value out of bounds.",
		 function );

		return( -1 );
	}
	if( io_handle->index_entry_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid IO handle - index entry size value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfsntfs_mft_attribute_get_data_flags(
	     mft_attribute,
	     &attribute_data_flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve attribute data flags.",
		 function );

		goto on_error;
	}
	if( ( attribute_data_flags & LIBFSNTFS_ATTRIBUTE_FLAG_COMPRESSION_MASK ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported compressed attribute data.",
		 function );

		goto on_error;
	}
	if( libfsntfs_mft_attribute_get_allocated_data_size(
	     mft_attribute,
	     &stored_allocated_data_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve attribute allocated data size.",
		 function );

		goto on_error;
	}
	if( libfdata_vector_initialize(
	     &safe_index_entry_vector,
	     (size64_t) io_handle->index_entry_size,
	     NULL,
	     NULL,
	     NULL,
	     (int (*)(intptr_t *, intptr_t *, libfdata_vector_t *, libfdata_cache_t *, int, int, off64_t, size64_t, uint32_t, uint8_t, libcerror_error_t **)) &libfsntfs_index_entry_vector_read_element_data,
	     NULL,
	     LIBFDATA_DATA_HANDLE_FLAG_NON_MANAGED,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create index entry vector.",
		 function );

		goto on_error;
	}
	while( mft_attribute != NULL )
	{
		if( libfsntfs_mft_attribute_get_data_vcn_range(
		     mft_attribute,
		     (uint64_t *) &attribute_data_vcn_offset,
		     (uint64_t *) &attribute_data_vcn_size,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve attribute data VCN range.",
			 function );

			goto on_error;
		}
		if( attribute_data_vcn_size != 0xffffffffffffffffULL )
		{
			if( (uint64_t) attribute_data_vcn_offset > (uint64_t) ( ( INT64_MAX / io_handle->cluster_block_size ) - 1 ) )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: invalid attribute data first VCN value out of bounds.",
				 function );

				goto on_error;
			}
			if( attribute_data_vcn_size > (size64_t) ( ( INT64_MAX / io_handle->cluster_block_size ) - 1 ) )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: invalid attribute data last VCN value out of bounds.",
				 function );

				goto on_error;
			}
			if( attribute_data_vcn_offset > (off64_t) attribute_data_vcn_size )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: invalid attribute data first VCN value out of bounds.",
				 function );

				goto on_error;
			}
			attribute_data_vcn_size   += 1;
			attribute_data_vcn_size   -= attribute_data_vcn_offset;
			attribute_data_vcn_offset *= io_handle->cluster_block_size;
			attribute_data_vcn_size   *= io_handle->cluster_block_size;

			if( ( calculated_attribute_data_vcn_offset != 0 )
			 && ( calculated_attribute_data_vcn_offset != attribute_data_vcn_offset ) )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
				 "%s: invalid attribute data VCN offset value out of bounds.",
				 function );

				goto on_error;
			}
			calculated_attribute_data_vcn_offset = attribute_data_vcn_offset + (off64_t) attribute_data_vcn_size;
		}
		if( libfsntfs_mft_attribute_get_number_of_data_runs(
		     mft_attribute,
		     &number_of_entries,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve attribute: %d number of data runs.",
			 function,
			 attribute_index );

			goto on_error;
		}
		for( entry_index = 0;
		     entry_index < number_of_entries;
		     entry_index++ )
		{
			if( libfsntfs_mft_attribute_get_data_run_by_index(
			     mft_attribute,
			     entry_index,
			     &data_run,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve attribute: %d data run: %d.",
				 function,
				 attribute_index,
				 entry_index );

				goto on_error;
			}
			if( data_run == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				 "%s: missing attribute: %d data run: %d.",
				 function,
				 attribute_index,
				 entry_index );

				goto on_error;
			}
			if( libfdata_vector_append_segment(
			     safe_index_entry_vector,
			     &segment_index,
			     0,
			     data_run->start_offset,
			     data_run->size,
			     0,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append attribute: %d data run: %d vector segment.",
				 function,
				 attribute_index,
				 entry_index );

				goto on_error;
			}
			calculated_allocated_data_size += data_run->size;
		}
		attribute_index++;

		if( libfsntfs_mft_attribute_get_next_attribute(
		     mft_attribute,
		     &mft_attribute,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve next MFT attribute: %d.",
			 function,
			 attribute_index );

			goto on_error;
		}
	}
	if( calculated_allocated_data_size != stored_allocated_data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: size of data runs: %" PRIu64 " does not match allocated data size: %" PRIu64 ".",
		 function,
		 calculated_allocated_data_size,
		 stored_allocated_data_size );

		goto on_error;
	}
	*index_entry_vector = safe_index_entry_vector;

	return( 1 );

on_error:
	if( safe_index_entry_vector != NULL )
	{
		libfdata_vector_free(
		 &safe_index_entry_vector,
		 NULL );
	}
	return( -1 );
}

/* Reads the index entry
 * Callback function for the index entry vector
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_index_entry_vector_read_element_data(
     intptr_t *data_handle LIBFSNTFS_ATTRIBUTE_UNUSED,
     libbfio_handle_t *file_io_handle,
     libfdata_vector_t *vector,
     libfdata_cache_t *cache,
     int element_index,
     int element_data_file_index LIBFSNTFS_ATTRIBUTE_UNUSED,
     off64_t index_entry_offset,
     size64_t index_entry_size,
     uint32_t element_flags LIBFSNTFS_ATTRIBUTE_UNUSED,
     uint8_t read_flags LIBFSNTFS_ATTRIBUTE_UNUSED,
     libcerror_error_t **error )
{
	libfsntfs_index_entry_t *index_entry = NULL;
	static char *function                = "libfsntfs_index_entry_vector_read_element_data";

	LIBFSNTFS_UNREFERENCED_PARAMETER( data_handle )
	LIBFSNTFS_UNREFERENCED_PARAMETER( element_data_file_index )
	LIBFSNTFS_UNREFERENCED_PARAMETER( element_flags )
	LIBFSNTFS_UNREFERENCED_PARAMETER( read_flags )

	if( (uint64_t) element_index > (uint64_t) UINT32_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid element index value out of bounds.",
		 function );

		return( -1 );
	}
	if( (uint64_t) index_entry_size > (uint64_t) UINT32_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid index entry size value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfsntfs_index_entry_initialize(
	     &index_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create index entry.",
		 function );

		goto on_error;
	}
	if( libfsntfs_index_entry_read_file_io_handle(
	     index_entry,
	     file_io_handle,
	     index_entry_offset,
	     (uint32_t) index_entry_size,
	     (uint32_t) element_index,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read index entry: %d at offset: %" PRIi64 " (0x%08" PRIx64 ").",
		 function,
		 element_index,
		 index_entry_offset,
		 index_entry_offset );

		goto on_error;
	}
	if( libfdata_vector_set_element_value_by_index(
	     vector,
	     (intptr_t *) file_io_handle,
	     cache,
	     element_index,
	     (intptr_t *) index_entry->node,
	     (int (*)(intptr_t **, libcerror_error_t **)) &libfsntfs_index_node_free,
	     LIBFDATA_LIST_ELEMENT_VALUE_FLAG_MANAGED,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set index node as element value.",
		 function );

		goto on_error;
	}
	index_entry->node = NULL;

	if( libfsntfs_index_entry_free(
	     &index_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free index entry.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( index_entry != NULL )
	{
		libfsntfs_index_entry_free(
		 &index_entry,
		 NULL );
	}
	return( -1 );
}

