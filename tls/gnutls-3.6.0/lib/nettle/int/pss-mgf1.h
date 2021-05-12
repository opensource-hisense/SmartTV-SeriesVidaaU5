/* pss-mgf1.h

   PKCS#1 mask generation function 1, used in RSA-PSS (RFC-3447).

   Copyright (C) 2017 Daiki Ueno

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#ifndef NETTLE_PSS_MGF1_H_INCLUDED
#define NETTLE_PSS_MGF1_H_INCLUDED

#include <nettle/nettle-meta.h>

#include <nettle/sha1.h>
#include <nettle/sha2.h>

#include <alloca.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Temporary allocation, for systems that don't support alloca. Note
 * that the allocation requests should always be reasonably small, so
 * that they can fit on the stack. For non-alloca systems, we use a
 * fix maximum size, and abort if we ever need anything larger. */

#if HAVE_ALLOCA
# define TMP_DECL(name, type, max) type *name
# define TMP_ALLOC(name, size) (name = alloca(sizeof (*name) * (size)))
#else /* !HAVE_ALLOCA */
# define TMP_DECL(name, type, max) type name[max]
# define TMP_ALLOC(name, size) \
  do { if ((size) > (sizeof(name) / sizeof(name[0]))) abort(); } while (0)
#endif

/* Arbitrary limits which apply to systems that don't have alloca */
#define NETTLE_MAX_HASH_BLOCK_SIZE 128
#define NETTLE_MAX_HASH_DIGEST_SIZE 64
#define NETTLE_MAX_HASH_CONTEXT_SIZE (sizeof(struct sha3_224_ctx))
#define NETTLE_MAX_SEXP_ASSOC 17
#define NETTLE_MAX_CIPHER_BLOCK_SIZE 32

/* Namespace mangling */
#define pss_mgf1 nettle_pss_mgf1

void
pss_mgf1(const void *seed, const struct nettle_hash *hash,
	 size_t length, uint8_t *mask);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_PSS_MGF1_H_INCLUDED */
