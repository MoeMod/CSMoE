/*
gl_drawmode.h
Copyright (C) 2024 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#pragma once

#include <vector>
#include <numeric>

namespace xe
{
	inline size_t genIotaIndices(std::vector<unsigned short> &index_buffer, int vertex_num) {
		index_buffer.resize(vertex_num);
		//for (int i = 0; i < vertex_buffer.size() - 2; i+=3) {
		//	index_buffer.emplace_back(0 + i);
		//	index_buffer.emplace_back(1 + i);
		//	index_buffer.emplace_back(2 + i);
		//}
		std::iota(index_buffer.begin(), index_buffer.end(), 0);
		return index_buffer.size();
	}

	inline size_t genQuadsIndices(std::vector<unsigned short> &index_buffer, int vertex_num)
	{
		index_buffer.clear();
		for (int i = 0; i < vertex_num - 3; i+=4) {
			index_buffer.emplace_back(0 + i);
			index_buffer.emplace_back(1 + i);
			index_buffer.emplace_back(2 + i);

			index_buffer.emplace_back(0 + i);
			index_buffer.emplace_back(2 + i);
			index_buffer.emplace_back(3 + i);
		}
		return index_buffer.size();
	}

	inline size_t genTriangleStripIndices(std::vector<unsigned short> &index_buffer, int vertex_num)
	{
		index_buffer.clear();
		for (int i = 2; i < vertex_num; ++i) {
			if( i & 1 )
			{
				// draw triangle [n-1 n-2 n]
				index_buffer.emplace_back(i - 1);
				index_buffer.emplace_back(i - 2);
				index_buffer.emplace_back(i);
			}
			else
			{
				// draw triangle [n-2 n-1 n]
				index_buffer.emplace_back(i - 2);
				index_buffer.emplace_back(i - 1);
				index_buffer.emplace_back(i);
			}
		}
		return index_buffer.size();
	}

	inline size_t genTriangleFanIndices(std::vector<unsigned short> &index_buffer, int vertex_num)
	{
		index_buffer.clear();
		for (int i = 2; i < vertex_num; ++i) {
			// draw triangle [0 n-1 n]
			index_buffer.emplace_back(0);
			index_buffer.emplace_back(i - 1);
			index_buffer.emplace_back(i);
		}
		return index_buffer.size();
	}

	inline size_t genQuadsStripIndices(std::vector<unsigned short> &index_buffer, int vertex_num)
	{
		index_buffer.clear();
		for (int i = 2; i < vertex_num - 1; i+=2) {
			index_buffer.emplace_back(i - 2);
			index_buffer.emplace_back(i - 1);
			index_buffer.emplace_back(i);

			index_buffer.emplace_back(i - 2);
			index_buffer.emplace_back(i);
			index_buffer.emplace_back(i + 1);
		}
		return index_buffer.size();
	}

	inline size_t genPolygonIndices(std::vector<unsigned short> &index_buffer, int vertex_num)
	{
		return genTriangleFanIndices(index_buffer, vertex_num);
	}
}