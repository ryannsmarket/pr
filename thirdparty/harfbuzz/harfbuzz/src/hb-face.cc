/*
 * Copyright © 2009  Red Hat, Inc.
 * Copyright © 2012  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */

#include "hb.hh"

#include "hb-face.hh"
#include "hb-blob.hh"
#include "hb-open-file.hh"
#include "hb-ot-face.hh"
#include "hb-ot-cmap-table.hh"
#include "hb-map.hh"


/**
 * SECTION:hb-face
 * @title: hb-face
 * @short_description: Font face objects
 * @include: hb.h
 *
 * A font face is an object that represents a single face from within a
 * font family.
 *
 * More precisely, a font face represents a single face in a binary font file.
 * Font faces are typically built from a binary blob and a face index.
 * Font faces are used to create fonts.
 **/


/**
 * hb_face_count:
 * @blob: a blob.
 *
 * Fetches the number of faces in a blob.
 *
 * Return value: Number of faces in @blob
 *
 * Since: 1.7.7
 **/
unsigned int
hb_face_count (hb_blob_t *blob)
{
  if (unlikely (!blob))
    return 0;

  /* TODO We shouldn't be sanitizing blob.  Port to run sanitizer and return if not sane. */
  /* Make API signature const after. */
  hb_blob_t *sanitized = hb_sanitize_context_t ().sanitize_blob<OT::OpenTypeFontFile> (hb_blob_reference (blob));
  const OT::OpenTypeFontFile& ot = *sanitized->as<OT::OpenTypeFontFile> ();
  unsigned int ret = ot.get_face_count ();
  hb_blob_destroy (sanitized);

  return ret;
}

/*
 * hb_face_t
 */

DEFINE_NULL_INSTANCE (hb_face_t) =
{
  HB_OBJECT_HEADER_STATIC,

  nullptr, /* reference_table_func */
  nullptr, /* user_data */
  nullptr, /* destroy */

  0,    /* index */
  1000, /* upem */
  0,    /* num_glyphs */

  /* Zero for the rest is fine. */
};


/**
 * hb_face_create_for_tables:
 * @reference_table_func: (closure user_data) (destroy destroy) (scope notified): Table-referencing function
 * @user_data: A pointer to the user data
 * @destroy: (nullable): A callback to call when @data is not needed anymore
 *
 * Variant of hb_face_create(), built for those cases where it is more
 * convenient to provide data for individual tables instead of the whole font
 * data. With the caveat that hb_face_get_table_tags() does not currently work
 * with faces created this way.
 *
 * Creates a new face object from the specified @user_data and @reference_table_func,
 * with the @destroy callback.
 *
 * Return value: (transfer full): The new face object
 *
 * Since: 0.9.2
 **/
hb_face_t *
hb_face_create_for_tables (hb_reference_table_func_t  reference_table_func,
			   void                      *user_data,
			   hb_destroy_func_t          destroy)
{
  hb_face_t *face;

  if (!reference_table_func || !(face = hb_object_create<hb_face_t> ())) {
    if (destroy)
      destroy (user_data);
    return hb_face_get_empty ();
  }

  face->reference_table_func = reference_table_func;
  face->user_data = user_data;
  face->destroy = destroy;

  face->num_glyphs.set_relaxed (-1);

  face->data.init0 (face);
  face->table.init0 (face);

  return face;
}


typedef struct hb_face_for_data_closure_t {
  hb_blob_t *blob;
  uint16_t  index;
} hb_face_for_data_closure_t;

static hb_face_for_data_closure_t *
_hb_face_for_data_closure_create (hb_blob_t *blob, unsigned int index)
{
  hb_face_for_data_closure_t *closure;

  closure = (hb_face_for_data_closure_t *) hb_calloc (1, sizeof (hb_face_for_data_closure_t));
  if (unlikely (!closure))
    return nullptr;

  closure->blob = blob;
  closure->index = (uint16_t) (index & 0xFFFFu);

  return closure;
}

static void
_hb_face_for_data_closure_destroy (void *data)
{
  hb_face_for_data_closure_t *closure = (hb_face_for_data_closure_t *) data;

  hb_blob_destroy (closure->blob);
  hb_free (closure);
}

static hb_blob_t *
_hb_face_for_data_reference_table (hb_face_t *face HB_UNUSED, hb_tag_t tag, void *user_data)
{
  hb_face_for_data_closure_t *data = (hb_face_for_data_closure_t *) user_data;

  if (tag == HB_TAG_NONE)
    return hb_blob_reference (data->blob);

  const OT::OpenTypeFontFile &ot_file = *data->blob->as<OT::OpenTypeFontFile> ();
  unsigned int base_offset;
  const OT::OpenTypeFontFace &ot_face = ot_file.get_face (data->index, &base_offset);

  const OT::OpenTypeTable &table = ot_face.get_table_by_tag (tag);

  hb_blob_t *blob = hb_blob_create_sub_blob (data->blob, base_offset + table.offset, table.length);

  return blob;
}

/**
 * hb_face_create:
 * @blob: #hb_blob_t to work upon
 * @index: The index of the face within @blob
 *
 * Constructs a new face object from the specified blob and
 * a face index into that blob.
 *
 * The face index is used for blobs of file formats such as TTC and
 * and DFont that can contain more than one face.  Face indices within
 * such collections are zero-based.
 *
 * <note>Note: If the blob font format is not a collection, @index
 * is ignored.  Otherwise, only the lower 16-bits of @index are used.
 * The unmodified @index can be accessed via hb_face_get_index().</note>
 *
 * <note>Note: The high 16-bits of @index, if non-zero, are used by
 * hb_font_create() to load named-instances in variable fonts.  See
 * hb_font_create() for details.</note>
 *
 * Return value: (transfer full): The new face object
 *
 * Since: 0.9.2
 **/
hb_face_t *
hb_face_create (hb_blob_t    *blob,
		unsigned int  index)
{
  hb_face_t *face;

  if (unlikely (!blob))
    blob = hb_blob_get_empty ();

  blob = hb_sanitize_context_t ().sanitize_blob<OT::OpenTypeFontFile> (hb_blob_reference (blob));

  hb_face_for_data_closure_t *closure = _hb_face_for_data_closure_create (blob, index);

  if (unlikely (!closure))
  {
    hb_blob_destroy (blob);
    return hb_face_get_empty ();
  }

  face = hb_face_create_for_tables (_hb_face_for_data_reference_table,
				    closure,
				    _hb_face_for_data_closure_destroy);

  face->index = index;

  return face;
}

/**
 * hb_face_get_empty:
 *
 * Fetches the singleton empty face object.
 *
 * Return value: (transfer full): The empty face object
 *
 * Since: 0.9.2
 **/
hb_face_t *
hb_face_get_empty ()
{
  return const_cast<hb_face_t *> (&Null (hb_face_t));
}


/**
 * hb_face_reference: (skip)
 * @face: A face object
 *
 * Increases the reference count on a face object.
 *
 * Return value: The @face object
 *
 * Since: 0.9.2
 **/
hb_face_t *
hb_face_reference (hb_face_t *face)
{
  return hb_object_reference (face);
}

/**
 * hb_face_destroy: (skip)
 * @face: A face object
 *
 * Decreases the reference count on a face object. When the
 * reference count reaches zero, the face is destroyed,
 * freeing all memory.
 *
 * Since: 0.9.2
 **/
void
hb_face_destroy (hb_face_t *face)
{
  if (!hb_object_destroy (face)) return;

  for (hb_face_t::plan_node_t *node = face->shape_plans; node; )
  {
    hb_face_t::plan_node_t *next = node->next;
    hb_shape_plan_destroy (node->shape_plan);
    hb_free (node);
    node = next;
  }

  face->data.fini ();
  face->table.fini ();

  if (face->destroy)
    face->destroy (face->user_data);

  hb_free (face);
}

/**
 * hb_face_set_user_data: (skip)
 * @face: A face object
 * @key: The user-data key to set
 * @data: A pointer to the user data
 * @destroy: (nullable): A callback to call when @data is not needed anymore
 * @replace: Whether to replace an existing data with the same key
 *
 * Attaches a user-data key/data pair to the given face object.
 *
 * Return value: `true` if success, `false` otherwise
 *
 * Since: 0.9.2
 **/
hb_bool_t
hb_face_set_user_data (hb_face_t          *face,
		       hb_user_data_key_t *key,
		       void *              data,
		       hb_destroy_func_t   destroy,
		       hb_bool_t           replace)
{
  return hb_object_set_user_data (face, key, data, destroy, replace);
}

/**
 * hb_face_get_user_data: (skip)
 * @face: A face object
 * @key: The user-data key to query
 *
 * Fetches the user data associated with the specified key,
 * attached to the specified face object.
 *
 * Return value: (transfer none): A pointer to the user data
 *
 * Since: 0.9.2
 **/
void *
hb_face_get_user_data (hb_face_t          *face,
		       hb_user_data_key_t *key)
{
  return hb_object_get_user_data (face, key);
}

/**
 * hb_face_make_immutable:
 * @face: A face object
 *
 * Makes the given face object immutable.
 *
 * Since: 0.9.2
 **/
void
hb_face_make_immutable (hb_face_t *face)
{
  if (hb_object_is_immutable (face))
    return;

  hb_object_make_immutable (face);
}

/**
 * hb_face_is_immutable:
 * @face: A face object
 *
 * Tests whether the given face object is immutable.
 *
 * Return value: `true` is @face is immutable, `false` otherwise
 *
 * Since: 0.9.2
 **/
hb_bool_t
hb_face_is_immutable (const hb_face_t *face)
{
  return hb_object_is_immutable (face);
}


/**
 * hb_face_reference_table:
 * @face: A face object
 * @tag: The #hb_tag_t of the table to query
 *
 * Fetches a reference to the specified table within
 * the specified face.
 *
 * Return value: (transfer full): A pointer to the @tag table within @face
 *
 * Since: 0.9.2
 **/
hb_blob_t *
hb_face_reference_table (const hb_face_t *face,
			 hb_tag_t tag)
{
  if (unlikely (tag == HB_TAG_NONE))
    return hb_blob_get_empty ();

  return face->reference_table (tag);
}

/**
 * hb_face_reference_blob:
 * @face: A face object
 *
 * Fetches a pointer to the binary blob that contains the
 * specified face. Returns an empty blob if referencing face data is not
 * possible.
 *
 * Return value: (transfer full): A pointer to the blob for @face
 *
 * Since: 0.9.2
 **/
hb_blob_t *
hb_face_reference_blob (hb_face_t *face)
{
  return face->reference_table (HB_TAG_NONE);
}

/**
 * hb_face_set_index:
 * @face: A face object
 * @index: The index to assign
 *
 * Assigns the specified face-index to @face. Fails if the
 * face is immutable.
 *
 * <note>Note: changing the index has no effect on the face itself
 * This only changes the value returned by hb_face_get_index().</note>
 *
 * Since: 0.9.2
 **/
void
hb_face_set_index (hb_face_t    *face,
		   unsigned int  index)
{
  if (hb_object_is_immutable (face))
    return;

  face->index = index;
}

/**
 * hb_face_get_index:
 * @face: A face object
 *
 * Fetches the face-index corresponding to the given face.
 *
 * <note>Note: face indices within a collection are zero-based.</note>
 *
 * Return value: The index of @face.
 *
 * Since: 0.9.2
 **/
unsigned int
hb_face_get_index (const hb_face_t *face)
{
  return face->index;
}

/**
 * hb_face_set_upem:
 * @face: A face object
 * @upem: The units-per-em value to assign
 *
 * Sets the units-per-em (upem) for a face object to the specified value.
 *
 * Since: 0.9.2
 **/
void
hb_face_set_upem (hb_face_t    *face,
		  unsigned int  upem)
{
  if (hb_object_is_immutable (face))
    return;

  face->upem.set_relaxed (upem);
}

/**
 * hb_face_get_upem:
 * @face: A face object
 *
 * Fetches the units-per-em (upem) value of the specified face object.
 *
 * Return value: The upem value of @face
 *
 * Since: 0.9.2
 **/
unsigned int
hb_face_get_upem (const hb_face_t *face)
{
  return face->get_upem ();
}

/**
 * hb_face_set_glyph_count:
 * @face: A face object
 * @glyph_count: The glyph-count value to assign
 *
 * Sets the glyph count for a face object to the specified value.
 *
 * Since: 0.9.7
 **/
void
hb_face_set_glyph_count (hb_face_t    *face,
			 unsigned int  glyph_count)
{
  if (hb_object_is_immutable (face))
    return;

  face->num_glyphs.set_relaxed (glyph_count);
}

/**
 * hb_face_get_glyph_count:
 * @face: A face object
 *
 * Fetches the glyph-count value of the specified face object.
 *
 * Return value: The glyph-count value of @face
 *
 * Since: 0.9.7
 **/
unsigned int
hb_face_get_glyph_count (const hb_face_t *face)
{
  return face->get_num_glyphs ();
}

/**
 * hb_face_get_table_tags:
 * @face: A face object
 * @start_offset: The index of first table tag to retrieve
 * @table_count: (inout): Input = the maximum number of table tags to return;
 *                Output = the actual number of table tags returned (may be zero)
 * @table_tags: (out) (array length=table_count): The array of table tags found
 *
 * Fetches a list of all table tags for a face, if possible. The list returned will
 * begin at the offset provided
 *
 * Return value: Total number of tables, or zero if it is not possible to list
 *
 * Since: 1.6.0
 **/
unsigned int
hb_face_get_table_tags (const hb_face_t *face,
			unsigned int  start_offset,
			unsigned int *table_count, /* IN/OUT */
			hb_tag_t     *table_tags /* OUT */)
{
  if (face->destroy != (hb_destroy_func_t) _hb_face_for_data_closure_destroy)
  {
    if (table_count)
      *table_count = 0;
    return 0;
  }

  hb_face_for_data_closure_t *data = (hb_face_for_data_closure_t *) face->user_data;

  const OT::OpenTypeFontFile &ot_file = *data->blob->as<OT::OpenTypeFontFile> ();
  const OT::OpenTypeFontFace &ot_face = ot_file.get_face (data->index);

  return ot_face.get_table_tags (start_offset, table_count, table_tags);
}


/*
 * Character set.
 */


#ifndef HB_NO_FACE_COLLECT_UNICODES
/**
 * hb_face_collect_unicodes:
 * @face: A face object
 * @out: The set to add Unicode characters to
 *
 * Collects all of the Unicode characters covered by @face and adds
 * them to the #hb_set_t set @out.
 *
 * Since: 1.9.0
 */
void
hb_face_collect_unicodes (hb_face_t *face,
			  hb_set_t  *out)
{
  face->table.cmap->collect_unicodes (out, face->get_num_glyphs ());
}
/**
 * hb_face_collect_variation_selectors:
 * @face: A face object
 * @out: The set to add Variation Selector characters to
 *
 * Collects all Unicode "Variation Selector" characters covered by @face and adds
 * them to the #hb_set_t set @out.
 *
 * Since: 1.9.0
 */
void
hb_face_collect_variation_selectors (hb_face_t *face,
				     hb_set_t  *out)
{
  face->table.cmap->collect_variation_selectors (out);
}
/**
 * hb_face_collect_variation_unicodes:
 * @face: A face object
 * @variation_selector: The Variation Selector to query
 * @out: The set to add Unicode characters to
 *
 * Collects all Unicode characters for @variation_selector covered by @face and adds
 * them to the #hb_set_t set @out.
 *
 * Since: 1.9.0
 */
void
hb_face_collect_variation_unicodes (hb_face_t *face,
				    hb_codepoint_t variation_selector,
				    hb_set_t  *out)
{
  face->table.cmap->collect_variation_unicodes (variation_selector, out);
}
#endif


/*
 * face-builder: A face that has add_table().
 */

struct hb_face_builder_data_t
{
  hb_hashmap_t<hb_tag_t, hb_blob_t *> tables;
};

static int compare_entries (const void* pa, const void* pb)
{
  const auto& a = * (const hb_pair_t<hb_tag_t, hb_blob_t*> *) pa;
  const auto& b = * (const hb_pair_t<hb_tag_t, hb_blob_t*> *) pb;

  /* Order by blob size first (smallest to largest) and then table tag */

  if (a.second->length != b.second->length)
    return a.second->length < b.second->length ? -1 : +1;

  return a.first < b.first ? -1 : a.first == b.first ? 0 : +1;
}

static hb_face_builder_data_t *
_hb_face_builder_data_create ()
{
  hb_face_builder_data_t *data = (hb_face_builder_data_t *) hb_calloc (1, sizeof (hb_face_builder_data_t));
  if (unlikely (!data))
    return nullptr;

  data->tables.init ();

  return data;
}

static void
_hb_face_builder_data_destroy (void *user_data)
{
  hb_face_builder_data_t *data = (hb_face_builder_data_t *) user_data;

  for (hb_blob_t* b : data->tables.values())
    hb_blob_destroy (b);

  data->tables.fini ();

  hb_free (data);
}

static hb_blob_t *
_hb_face_builder_data_reference_blob (hb_face_builder_data_t *data)
{

  unsigned int table_count = data->tables.get_population ();
  unsigned int face_length = table_count * 16 + 12;

  for (hb_blob_t* b : data->tables.values())
    face_length += hb_ceil_to_4 (hb_blob_get_length (b));

  char *buf = (char *) hb_malloc (face_length);
  if (unlikely (!buf))
    return nullptr;

  hb_serialize_context_t c (buf, face_length);
  c.propagate_error (data->tables);
  OT::OpenTypeFontFile *f = c.start_serialize<OT::OpenTypeFontFile> ();

  bool is_cff = (data->tables.has (HB_TAG ('C','F','F',' '))
                 || data->tables.has (HB_TAG ('C','F','F','2')));
  hb_tag_t sfnt_tag = is_cff ? OT::OpenTypeFontFile::CFFTag : OT::OpenTypeFontFile::TrueTypeTag;

  // Sort the tags so that produced face is deterministic.
  hb_vector_t<hb_pair_t <hb_tag_t, hb_blob_t*>> sorted_entries;
  data->tables.iter () | hb_sink (sorted_entries);
  if (unlikely (sorted_entries.in_error ()))
  {
    hb_free (buf);
    return nullptr;
  }

  sorted_entries.qsort (compare_entries);
  bool ret = f->serialize_single (&c, sfnt_tag, + sorted_entries.iter());

  c.end_serialize ();

  if (unlikely (!ret))
  {
    hb_free (buf);
    return nullptr;
  }

  return hb_blob_create (buf, face_length, HB_MEMORY_MODE_WRITABLE, buf, hb_free);
}

static hb_blob_t *
_hb_face_builder_reference_table (hb_face_t *face HB_UNUSED, hb_tag_t tag, void *user_data)
{
  hb_face_builder_data_t *data = (hb_face_builder_data_t *) user_data;

  if (!tag)
    return _hb_face_builder_data_reference_blob (data);

  return hb_blob_reference (data->tables[tag]);
}


/**
 * hb_face_builder_create:
 *
 * Creates a #hb_face_t that can be used with hb_face_builder_add_table().
 * After tables are added to the face, it can be compiled to a binary
 * font file by calling hb_face_reference_blob().
 *
 * Return value: (transfer full): New face.
 *
 * Since: 1.9.0
 **/
hb_face_t *
hb_face_builder_create ()
{
  hb_face_builder_data_t *data = _hb_face_builder_data_create ();
  if (unlikely (!data)) return hb_face_get_empty ();

  return hb_face_create_for_tables (_hb_face_builder_reference_table,
				    data,
				    _hb_face_builder_data_destroy);
}

/**
 * hb_face_builder_add_table:
 * @face: A face object created with hb_face_builder_create()
 * @tag: The #hb_tag_t of the table to add
 * @blob: The blob containing the table data to add
 *
 * Add table for @tag with data provided by @blob to the face.  @face must
 * be created using hb_face_builder_create().
 *
 * Since: 1.9.0
 **/
hb_bool_t
hb_face_builder_add_table (hb_face_t *face, hb_tag_t tag, hb_blob_t *blob)
{
  if (tag == HB_MAP_VALUE_INVALID)
    return false;

  if (unlikely (face->destroy != (hb_destroy_func_t) _hb_face_builder_data_destroy))
    return false;

  hb_face_builder_data_t *data = (hb_face_builder_data_t *) face->user_data;

  hb_blob_t* previous = data->tables.get (tag);
  if (!data->tables.set (tag, hb_blob_reference (blob)))
  {
    hb_blob_destroy (blob);
    return false;
  }

  hb_blob_destroy (previous);
  return true;
}
