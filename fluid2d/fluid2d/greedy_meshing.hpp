#ifndef __GREEDY_MESHING_HPP__
#define __GREEDY_MESHING_HPP__


#include <vector>
using namespace std;


struct Quad
{
	int x, y;
	int w, h;
	int type;
};


template<int N>
void GreedyMeshing(vector<Quad>& qu, int (*checked)[N], int (*block)[N], int size_w, int size_h)
{
	qu.clear();

	for (int x = 0; x < size_w; ++x)
		for (int y = 0; y < size_h; ++y) {
			checked[x][y] = 0;
		}

	// move check point, skip the air block
	for (int x = 0; x < size_w; ++x)
		for (int y = 0; y < size_h; ++y)
		{
			if (checked[x][y] == 1)  continue;
			if (blocks[x][y] == 0) {  
				checked[x][y] = 1;
				continue;
			}

			// check quad size
			Quad q;
			q.x = x, q.y = y;
			q.type = blocks[x][y];

			// make check point
			int check_x = x, check_y = y;
			
			// scan y direction
			while (q.type == blocks[check_x][check_y] && check_y < size_h) {
				checked[check_x][check_y] = 1;
				++check_y;
			}
			q.h = check_y - q.y;

			// extend quad to x direction
			do {
				check_x += 1;
				check_y = q.y;
				// scan y direction
				while (q.type == blocks[check_x][check_y] && check_y < q.y+q.h) {
					checked[check_x][check_y] = 1;
					++check_y;
				}
			} while (q.h == check_y - q.y && check_x < size_w);
			q.w = check_x - q.x;

			// if last scanline is not fininshed, need to restore the check states
			if( q.h != check_y - q.y ) {
				while (check_y > q.y) {
					--check_y;
					checked[check_x][check_y] = 0;
				}
			}

			// push to vector
			qu.push_back(q);
		}
}


#endif