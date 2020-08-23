#pragma once

#include <utility>
#include <iterator>

struct SDL_Rect;


struct [[nodiscard]] QUAD {
	int left;
	int top;
	int right;
	int bottom;

	static QUAD empty();
	static QUAD from_wh(int w, int h);
	static QUAD from_xywh(int x, int y, int w, int h);

	friend bool operator==(QUAD const&, QUAD const&);
	friend bool operator!=(QUAD const&, QUAD const&);

	bool is_empty() const;
	int width() const;
	int height() const;

	// expand to encompass the passed QUAD or point
	QUAD include_region(QUAD) const;
	QUAD include_point(int x, int y) const;

	// shrink to fit within boundary
	QUAD clip_to(QUAD boundary) const;

	SDL_Rect to_sdl() const;
};


/// Helpers to allow RECTs to be iterated over

struct [[nodiscard]] QUAD_ITERATOR {
	QUAD rect;
	std::pair<int, int> xy; // using a pair here bc C++

	QUAD_ITERATOR& operator++();
	QUAD_ITERATOR operator++(int);

	std::pair<int, int> const& operator*() const;
	std::pair<int, int> const* operator->() const;

	friend bool operator==(QUAD_ITERATOR const&, QUAD_ITERATOR const&);
	friend bool operator!=(QUAD_ITERATOR const&, QUAD_ITERATOR const&);
};

template<>
struct std::iterator_traits<QUAD_ITERATOR> {
	using iterator_category = std::input_iterator_tag;

	using difference_type = void;
	using value_type = std::pair<int, int>;
	using pointer = value_type const*;
	using reference = value_type const&;
};

QUAD_ITERATOR begin(QUAD);
QUAD_ITERATOR end(QUAD);