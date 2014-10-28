/* $Id: tilearea_type.h 24900 2013-01-08 22:46:42Z planetmaker $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tilearea_type.h Type for storing the 'area' of something uses on the map. */

/**
 * @defgroup TileIndexTransformations Tile index transformations
 *
 * @image html tile_index_transformations.svg
 */

#ifndef TILEAREA_TYPE_H
#define TILEAREA_TYPE_H

#include "map_func.h"
#include "direction_type.h"

template <bool Tgeneric>
struct TileAreaT;

/**
 * Set of coordinates representing rectangular piece of a tile map.
 *
 * This "raw" area does not point to any map. These are pure coordinates. Unless
 * we bound them to a cartain map we cannot make most of calculations.
 *
 * @see RawTileIndex
 * @see TileAreaT
 */
struct RawTileArea {
	RawTileIndex tile; ///< The base (northern) tile of the area
	uint16 w;          ///< The width of the area
	uint16 h;          ///< The height of the area
};

/**
 * Set of coordinates representing rectangular piece of a tile map.
 *
 * This tile area, based on template args, can represent a part of either the main map
 * or any chosen map.
 *
 * @tparam Tgeneric If \c false, this area will represent tiles on the main map.
 *                  If \c true, this area will be able to represent tiles on any map chosen on a run-tim.
 *
 * @note To use a specific overload there are TileArea and GenericTileArea to your
 *       disposition. Use TileAreaT type directly only if you work with templates.
 *
 * @see TileArea
 * @see GenericTileArea
 * @see RawTileArea
 * @see TileIndexT
 */
template <bool Tgeneric>
struct TileAreaT {
	typedef typename TileIndexT<Tgeneric>::T TileIndexType; ///< The type of tile indices, TileIndex or GenericTileIndex.

	TileIndexType tile; ///< The base (northern) tile of the area
	uint16 w;           ///< The width of the area
	uint16 h;           ///< The height of the area

	/** Just construct this tile area */
	TileAreaT() {}

	/**
	 * Make a copy of a given tile area
	 * @param ta the area to copy
	 */
	template <bool TotherGeneric>
	TileAreaT(const TileAreaT<TotherGeneric> &ta)
		: tile(MakeTileIndex<Tgeneric>(IndexOf(ta.tile), MapOf(ta.tile))), w(ta.w), h(ta.h)
	{
	}

	/**
	 * Construct this tile area from a "raw" tile area and a given tile map
	 * @param ta the "raw" tile area
	 * @param map the map
	 */
	inline TileAreaT(const RawTileArea &ta, Map *map)
		: tile(MakeTileIndex<Tgeneric>(ta.tile, map)), w(ta.w), h(ta.h)
	{
	}

	/**
	 * Construct this tile area with some set values
	 * @param tile the base tile
	 * @param w the width
	 * @param h the height
	 */
	TileAreaT(TileIndexType tile, uint8 w, uint8 h) : tile(tile), w(w), h(h) {}

	TileAreaT(TileIndexType start, TileIndexType end);


	void Add(TileIndexType to_add);

	/**
	 * Clears the 'tile area', i.e. make the tile invalid.
	 */
	void Clear()
	{
		IndexOf(this->tile) = INVALID_TILE_INDEX;
		this->w    = 0;
		this->h    = 0;
	}

	bool Intersects(const TileAreaT<Tgeneric> &ta) const;

	bool Contains(const TileAreaT<Tgeneric> &ta) const;

	bool Contains(TileIndexType tile) const;

	void ClampToMap();

	/**
	 * Get the center tile.
	 * @return The tile at the center, or just north of it.
	 */
	TileIndexType GetCenterTile() const
	{
		return TILE_ADDXY(this->tile, this->w / 2, this->h / 2);
	}

	TileIndexDiffC TransformedNorthOffset(DirTransformation transformation) const;
	TileIndexDiffC TransformedTileOffset(TileIndexType tile, DirTransformation transformation) const;

	/**
	 * Transform northern tile of this area based on a given northern tile of transformed area.
	 *
	 * @param dst_area_north Point of reference, the northern tile of the transformed area.
	 * @param transformation Transformation to perform.
	 * @return Northern tile of this area after transformation.
	 *
	 * @see TileIndexTransformations
	 *
	 * @ingroup TileIndexTransformations
	 */
	template <bool TgenericDst>
	typename TileIndexT<TgenericDst>::T TransformedNorth(typename TileIndexT<TgenericDst>::T dst_area_north, DirTransformation transformation) const
	{
		TileIndexDiffC offs = this->TransformedNorthOffset(transformation);
		return TILE_ADDXY(dst_area_north, offs.x, offs.y);
	}
	/** @copydoc TileAreaT::TransformedNorth(TileIndexT<TgenericDst>::T,DirTransformation) */
	inline TileIndex TransformedNorth(TileIndex dst_area_north, DirTransformation transformation) const { return this->TransformedNorth<false>(dst_area_north, transformation); }
	/** @copydoc TileAreaT::TransformedNorth(TileIndexT<TgenericDst>::T,DirTransformation) */
	inline GenericTileIndex TransformedNorth(GenericTileIndex dst_area_north, DirTransformation transformation) const { return this->TransformedNorth<true>(dst_area_north, transformation); }

	/**
	 * Calculate northern tile of transformed area based on transformed northern tile of this area.
	 *
	 * @param transformed_north Point of reference, northern tile of this area after transformation.
	 * @param transformation The transformation.
	 * @return Northern tile of the transformed area.
	 *
	 * @see TileIndexTransformations
	 *
	 * @ingroup TileIndexTransformations
	 */
	template <bool TgenericDst>
	typename TileIndexT<TgenericDst>::T ReverseTransformedNorth(typename TileIndexT<TgenericDst>::T transformed_north, DirTransformation transformation) const
	{
		TileIndexDiffC offs = this->TransformedNorthOffset(transformation);
		return TILE_ADDXY(transformed_north, -offs.x, -offs.y);
	}
	/** @copydoc TileAreaT::ReverseTransformedNorth(TileIndexT<TgenericDst>::T,DirTransformation) */
	inline TileIndex ReverseTransformedNorth(TileIndex transformed_north, DirTransformation transformation) const { return this->ReverseTransformedNorth<false>(transformed_north, transformation); }
	/** @copydoc TileAreaT::ReverseTransformedNorth(TileIndexT<TgenericDst>::T,DirTransformation) */
	inline GenericTileIndex ReverseTransformedNorth(GenericTileIndex transformed_north, DirTransformation transformation) const { return this->ReverseTransformedNorth<true>(transformed_north, transformation); }

	/**
	 * Transform a given tile within this area.
	 *
	 * @param tile The tile to transform.
	 * @param transformed_north Point of reference, northern tile of this area after transformation.
	 * @param transformation Transformation to perform.
	 * @return Transformed tile.
	 *
	 * @see TileIndexTransformations
	 *
	 * @ingroup TileIndexTransformations
	 */
	template <bool TgenericDst>
	typename TileIndexT<TgenericDst>::T TransformTile(TileIndexType tile, typename TileIndexT<TgenericDst>::T transformed_north, DirTransformation transformation) const
	{
		TileIndexDiffC offs = this->TransformedTileOffset(tile, transformation);
		return TILE_ADDXY(transformed_north, offs.x, offs.y);
	}
	/** @copydoc TileAreaT::TransformTile(TileIndexType,TileIndexT<TgenericDst>::T,DirTransformation) */
	inline TileIndex TransformTile(TileIndexType tile, TileIndex transformed_north, DirTransformation transformation) const { return this->TransformTile<false>(tile, transformed_north, transformation); }
	/** @copydoc TileAreaT::TransformTile(TileIndexType,TileIndexT<TgenericDst>::T,DirTransformation) */
	inline GenericTileIndex TransformTile(TileIndexType tile, GenericTileIndex transformed_north, DirTransformation transformation) const { return this->TransformTile<true>(tile, transformed_north, transformation); }

	/**
	 * Get the point of reference of a transfomation based on a given tile before and after transformation.
	 *
	 * @param tile The tile before transformation.
	 * @param transformed_tile The tile after transformation.
	 * @param transformation The transformation.
	 * @return The point of reference (northern tile of this area after transformation).
	 *
	 * @see TileIndexTransformations
	 *
	 * @ingroup TileIndexTransformations
	 */
	template <bool TgenericDst>
	typename TileIndexT<TgenericDst>::T ReverseTransformTile(TileIndexType source_tile, typename TileIndexT<TgenericDst>::T transformed_tile, DirTransformation transformation) const
	{
		TileIndexDiffC offs = this->TransformedTileOffset(source_tile, transformation);
		return TILE_ADDXY(transformed_tile, -offs.x, -offs.y);
	}
	/** @copydoc TileAreaT::ReverseTransformTile(TileIndexType,TileIndexT<TgenericDst>::T,DirTransformation) */
	inline TileIndex ReverseTransformTile(TileIndexType source_tile, TileIndex transformed_tile, DirTransformation transformation) const { return this->ReverseTransformTile<false>(source_tile, transformed_tile, transformation); }
	/** @copydoc TileAreaT::ReverseTransformTile(TileIndexType,TileIndexT<TgenericDst>::T,DirTransformation) */
	inline GenericTileIndex ReverseTransformTile(TileIndexType source_tile, GenericTileIndex transformed_tile, DirTransformation transformation) const { return this->ReverseTransformTile<true>(source_tile, transformed_tile, transformation); }
};

/**
 * Set of coordinates representing rectangular piece of the main tile map.
 *
 * This is the most common type of tile area. It represents tiles on the main tile array.
 *
 * @see TileIndex
 * @see GenericTileArea
 */
typedef TileAreaT<false> TileArea;

/**
 * Set of coordinates representing rectangular piece of a tile map.
 *
 * This "generic" tile area is able to represent part of any map chosen on a run-time.
 *
 * @see GenericTileIndex
 * @see TileArea
 */
typedef TileAreaT<true> GenericTileArea;

/** Base class for tile iterators. */
template <bool Tgeneric>
class TileIteratorT {
public:
	typedef typename TileIndexT<Tgeneric>::T TileIndexType; ///< The type of tile indices, TileIndex or GenericTileIndex.

protected:
	TileIndexType tile; ///< The current tile we are at.

	/**
	 * Initialise the iterator starting at this tile.
	 * @param tile The tile we start iterating from.
	 */
	TileIteratorT(TileIndexType tile) : tile(tile)
	{
	}

	/**
	 * Get the raw tile index of this iterator.
	 * @return Pointer to the index.
	 */
	inline RawTileIndex *MyIndex()
	{
		return &IndexOf(this->tile);
	}

	/**
	 * Get the map of this iterator.
	 * @return The map that this iterator iterates through.
	 */
	inline Map *MyMap() const
	{
		return MapOf(this->tile);
	}

public:
	/** Some compilers really like this. */
	virtual ~TileIteratorT()
	{
	}

	/**
	 * Get the tile we are currently at.
	 * @return The tile we are at, or "invalid" tile when we're done.
	 */
	inline operator TileIndexType () const
	{
		return this->tile;
	}

	/**
	 * Move ourselves to the next tile in the rectangle on the map.
	 */
	virtual TileIteratorT<Tgeneric>& operator ++() = 0;

	/**
	 * Allocate a new iterator that is a copy of this one.
	 */
	virtual TileIteratorT<Tgeneric> *Clone() const = 0;
};

/** Base class for tile iterators of the main map. */
typedef TileIteratorT<false> TileIterator;

/** Helper class to build diagonal tile iterators. */
class OrthogonalTileIteratorController {
public:
	int w; ///< The width of the iterated area.
	int x; ///< The current 'x' position in the rectangle.
	int y; ///< The current 'y' position in the rectangle.

	/**
	 * Initialize iteration.
	 * @param my_index Pointer to the tile index of the iterator. The index must be set to the first tile of the iteration before you call Init.
	 * @param w The width of the iterated area.
	 * @param h The height of the iterated area.
	 */
	void Init(RawTileIndex *my_index, uint w, uint h)
	{
		this->w = w;
		this->x = w;
		this->y = h;
		if (w == 0 || h == 0) *my_index = INVALID_TILE_INDEX;
	}

	/**
	 * Perform single iteration step.
	 * @param my_index Pointer to the tile index of the iterator.
	 * @param my_map The map that iterator iterates through.
	 */
	inline void Advance(RawTileIndex *my_index, Map *my_map)
	{
		assert(*my_index != INVALID_TILE_INDEX);

		if (--this->x > 0) {
			++*my_index;
		} else if (--this->y > 0) {
			this->x = this->w;
			*my_index += TileDiffXY(1, 1, my_map) - this->w;
		} else {
			*my_index = INVALID_TILE_INDEX;
		}
	}
};

/** Iterator to iterate over a tile area (rectangle) of a map. */
template <bool Tgeneric>
class OrthogonalTileIteratorT : public TileIteratorT<Tgeneric>, protected OrthogonalTileIteratorController {
public:
	typedef typename TileIteratorT<Tgeneric>::TileIndexType TileIndexType;

	/**
	 * Construct the iterator.
	 * @param ta Area, i.e. begin point and width/height of to-be-iterated area.
	 */
	OrthogonalTileIteratorT(const TileAreaT<Tgeneric> &ta) : TileIteratorT<Tgeneric>(ta.tile)
	{
		this->Init(this->MyIndex(), ta.w, ta.h);
	}

	/**
	 * Move ourselves to the next tile in the rectangle on the map.
	 */
	inline TileIteratorT<Tgeneric> &operator ++ ()
	{
		this->Advance(this->MyIndex(), this->MyMap());
		return *this;
	}

	virtual TileIteratorT<Tgeneric> *Clone() const
	{
		return new OrthogonalTileIteratorT<Tgeneric>(*this);
	}
};

/** Iterator to iterate over a tile area (rectangle) of the main map. */
typedef OrthogonalTileIteratorT<false> OrthogonalTileIterator;

/** Helper class to build diagonal tile iterators. */
class DiagonalTileIteratorController {
public:
	uint base_x; ///< The base tile x coordinate from where the iterating happens.
	uint base_y; ///< The base tile y coordinate from where the iterating happens.
	int a_cur;   ///< The current (rotated) x coordinate of the iteration.
	int b_cur;   ///< The current (rotated) y coordinate of the iteration.
	int a_max;   ///< The (rotated) x coordinate of the end of the iteration.
	int b_max;   ///< The (rotated) y coordinate of the end of the iteration.

	void Init(RawTileIndex *my_index, RawTileIndex opposite_corner, Map *my_map);
	void Advance(RawTileIndex *my_index, Map *my_map);
};

/** Iterator to iterate over a diagonal area of a map. */
template <bool Tgeneric>
class DiagonalTileIteratorT : public TileIteratorT<Tgeneric>, protected DiagonalTileIteratorController {
public:
	typedef typename TileIteratorT<Tgeneric>::TileIndexType TileIndexType;

	/**
	 * Construct the iterator.
	 * @param begin The first corner of the to-be-itarted area.
	 * @param end The second (opposite) corner of the to-be-itarted area.
	 */
	DiagonalTileIteratorT(TileIndexType begin, TileIndexType end) : TileIteratorT<Tgeneric>(begin)
	{
		assert(IsSameMap(begin, end));
		this->Init(this->MyIndex(), end, this->MyMap());
	}

	TileIteratorT<Tgeneric> &operator ++ ()
	{
		this->Advance(this->MyIndex(), this->MyMap());
		return *this;
	}

	virtual TileIteratorT<Tgeneric> *Clone() const
	{
		return new DiagonalTileIteratorT<Tgeneric>(*this);
	}
};

/** Iterator to iterate over a diagonal area of the main map. */
typedef DiagonalTileIteratorT<false> DiagonalTileIterator;

/** Helper class to build transformative tile iterators. */
class TransformationTileIteratorController : public OrthogonalTileIteratorController {
public:
	DirTransformation transformation; ///< Transformation to perform.

	void Init(RawTileIndex *src_index, RawTileIndex *dst_index, uint16 src_w, uint16 src_h, DirTransformation transformation);
	void Advance(RawTileIndex *src_index, Map *src_map, RawTileIndex *dst_index, Map *dst_map);
};

/**
 * Iterator to iterate over a diagonal area of a map performing transformation on tile indices.
 *
 * Iterator will iterate over source area in the same way OrthogonalTileIteratorT do, additionally
 * performing transformation on tile indices. You can call SrcTile or DstTile to get the tile before
 * and after transformation.
 *
 * The tile of this iterator (it's base) is the transformed one.
 */
template <bool TgenericSrc, bool TgenericDst>
class TransformationTileIteratorT : public TileIteratorT<TgenericDst>, protected TransformationTileIteratorController {
public:
	typedef typename TileIteratorT<TgenericDst>::TileIndexType TileIndexType;
	typedef typename TileIndexT<TgenericDst>::T DstTileIndexType;
	typedef typename TileIndexT<TgenericSrc>::T SrcTileIndexType;

protected:
	SrcTileIndexType src_tile; ///< Current tile of the source area.

public:
	/**
	 * Create a TransformationTileIteratorT.
	 *
	 * @param src_area Source area to be transformed and iterated over.
	 * @param transformed_north Transformed northern tile of the source area.
	 * @param transformation Transformation to perform.
	 */
	TransformationTileIteratorT(const TileAreaT<TgenericSrc> &src_area, DstTileIndexType transformed_north, DirTransformation transformation)
		: TileIteratorT<TgenericDst>(transformed_north), src_tile(src_area.tile)
	{
		this->Init(&IndexOf(this->src_tile), this->MyIndex(), src_area.w, src_area.h, transformation);
	}

	/**
	 * The source tile of the transformation.
	 * @return Tile before transformation.
	 */
	inline const SrcTileIndexType &SrcTile() const { return this->src_tile; }

	/**
	 * The destination tile of the transformation (the tile of this iterator).
	 * @return Tile after transformation.
	 */
	inline const DstTileIndexType &DstTile() const { return this->tile; }

	virtual TileIteratorT<TgenericDst> &operator ++ ()
	{
		this->Advance(&IndexOf(this->src_tile), MapOf(this->src_tile), this->MyIndex(), this->MyMap());
		return *this;
	}

	virtual TileIteratorT<TgenericDst> *Clone() const
	{
		return new TransformationTileIteratorT<TgenericSrc, TgenericDst>(*this);
	}
};

/** Iterator to iterate over a diagonal area of the main map performing transformation on tile indices. */
typedef TransformationTileIteratorT<false, false> TransformationTileIterator;

/**
 * A loop which iterates over the tiles of a TileArea.
 * @param var The name of the variable which contains the current tile.
 *            This variable will be allocated in this \c for of this loop.
 * @param ta  The tile area to search over.
 */
#define TILE_AREA_LOOP(var, ta) for (OrthogonalTileIterator var(ta); var != INVALID_TILE; ++var)

/**
 * A loop which iterates over the tiles of a GenericTileArea.
 * @param var The name of the variable which contains the current tile.
 *            This variable will be allocated in this \c for of this loop.
 * @param ta  The tile area to search over.
 */
#define GENERIC_TILE_AREA_LOOP(var, ta) for (OrthogonalTileIteratorT<true> var(ta); IsValidTileIndex<true>(var); ++var)

#endif /* TILEAREA_TYPE_H */
