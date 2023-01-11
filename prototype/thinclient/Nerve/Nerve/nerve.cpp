// Nerve
// by Marc A. Lepage 2007-05-13

/*

NERVE PROTOCOL

- tiles come in types (by code)
- major tile has N1 vertices, minor tile has N2 vertices
- vertices are written in order as 8 byte big endian double precision floating point numbers

*/

// standard includes
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// boost includes
#include <boost/asio.hpp>
#include <boost/filesystem/operations.hpp>

// pyxlib includes
#include "pyxlib_instance.h"
#include "pyxis/data/coverage.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/string_utils.h"

#define WRITE_XYZ 0
#define WRITE_TEX 1

using namespace std;
using boost::asio::ip::tcp;

template <typename T>
string tostr(T t)
{
	ostringstream ost;
	ost << t;
	return ost.str();
}

typedef signed char cmd_t;
const cmd_t CMD_EXIT		= 1;
const cmd_t CMD_TILESET		= 10;
const cmd_t CMD_TILECOORD	= 11;
const cmd_t CMD_TEXTURE		= 12;

boost::filesystem::path cachedir;

fstream file;

//! Vector of resolution 2 indices with which to begin traversal.
std::vector<PYXIcosIndex> vecRes2;

const int knInside = -1;
const int knBoundary = 0;
const int knOutside = 1;

const SnyderProjection* pSnyder;
int tmpInt;
double tmpNum;
std::string tmpStr;
PYXIcosIndex tmpIndex;
PYXCoord3DDouble tmpCoord;

double m[16];
unsigned char rgb;

boost::intrusive_ptr<ICoverage> spCov;

// Only for first column of texture quad strips. The other two columns are obtained by rotation.
// Format:
// count of quads (terminated by 0)
// series of cursor commands (terminated by .) to go to top left of strip facing top right
// for each quad, a rotation from the current next row direction (starting with next column direction) to yield the next
const char* const tex_spec[] =
{
	"",

	"",

	"3 F N . -1 0 -1 "
	"5 L F L F R . -2 0 1 -1 1 "
	"3 F F L F L L . -1 0 -1 "
	"1 F L F F L . -1 "
	"1 F F F L L . -2 "
	"0",

	"3 F N . -1 0 -1 "
	"3 L F L F R . -2 0 1 "
	"1 F L F L . -2 "
	"0"
};

bool isTileMajor(const string& name)
{
	return name[name.length() - 1] == '0';
}

int getTileClass(const string& name)
{
	return 2 - (name.length() - (name[2] == '-' ? 1 : 0)) % 2;
}

int getTileCode(const string& name)
{
	bool bMajor = isTileMajor(name);
	int nClass = getTileClass(name);
	return (nClass - 1) * 2 + (bMajor ? 0 : 1);
}

void init_nerve()
{
	pSnyder = SnyderProjection::getInstance();

	PYXIcosIterator it(2);
	for (; !it.end(); it.next())
	{
		vecRes2.push_back(it.getIndex());
	}
	std::reverse(vecRes2.begin(), vecRes2.end());

	cachedir = "nerve_cache";
	if (!exists(cachedir))
	{
		create_directory(cachedir);
	}

	boost::intrusive_ptr<IProcess> spProc =
		PipeManager::getProcess(strToProcRef("{C10C1C95-CBD3-4073-9C32-791FF1464D9B}[1]"));
	spProc->QueryInterface(ICoverage::iid, (void**) &spCov);
	assert(spCov);

	cout << "nerve ready\n";
}

void writeDouble(tcp::iostream& stream, double f)
{
	char buf[8];
	char* p = reinterpret_cast<char*>(&f);
	buf[0] = p[7];
	buf[1] = p[6];
	buf[2] = p[5];
	buf[3] = p[4];
	buf[4] = p[3];
	buf[5] = p[2];
	buf[6] = p[1];
	buf[7] = p[0];
	stream.write(buf, 8);
}

void writeInt(tcp::iostream& stream, int n)
{
	char buf[4];
	char* p = reinterpret_cast<char*>(&n);
	buf[0] = p[3];
	buf[1] = p[2];
	buf[2] = p[1];
	buf[3] = p[0];
	stream.write(buf, 4);
}

void stream_coord(tcp::iostream& stream, const PYXIcosIndex& index)
{
	pSnyder->pyxisToXYZ(index, &tmpCoord);
//	stream
//		<< tostr(tmpCoord.x()).c_str() << ' '
//		<< tostr(tmpCoord.y()).c_str() << ' '
//		<< tostr(tmpCoord.z()).c_str() << ' ';
	writeDouble(stream, tmpCoord.x());
	writeDouble(stream, tmpCoord.y());
	writeDouble(stream, tmpCoord.z());
#if WRITE_XYZ
	char buf[8];
	double val[3] = { tmpCoord.x(), tmpCoord.y(), tmpCoord.z() };
	for (int nV = 0; nV != 3; ++nV)
	{
		// convert to big endian
		char* p = reinterpret_cast<char*>(val + nV);
		buf[0] = p[7];
		buf[1] = p[6];
		buf[2] = p[5];
		buf[3] = p[4];
		buf[4] = p[3];
		buf[5] = p[2];
		buf[6] = p[1];
		buf[7] = p[0];
		file.write(buf, 8);
	}
	file
		<< tostr(tmpCoord.x()).c_str() << ' '
		<< tostr(tmpCoord.y()).c_str() << ' '
		<< tostr(tmpCoord.z()).c_str() << ' ';
#endif
}

int classify_point_to_frustrum(const PYXIcosIndex& index)
{
	pSnyder->pyxisToXYZ(index, &tmpCoord);

	//double x = m[0] * tmpCoord.x() + m[1] * tmpCoord.y() + m[2]  * tmpCoord.z();
	//double y = m[4] * tmpCoord.x() + m[5] * tmpCoord.y() + m[6]  * tmpCoord.z();
	double z = m[8] * tmpCoord.x() + m[9] * tmpCoord.y() + m[10] * tmpCoord.z();

	double dot = z;

	if (0 < dot)
	{
		return knInside;
	}
	else if (dot < 0)
	{
		return knOutside;
	}

	return knBoundary;
}

int classify_cell_to_frustrum(const PYXIcosIndex& index)
{
	// TODO
	return classify_point_to_frustrum(index);
}

void cmd_snyder(tcp::iostream& stream)
{
	stream >> tmpInt;

	for (int n = 0; n != tmpInt; ++n)
	{
		stream >> tmpStr;
		tmpIndex = tmpStr;
		pSnyder->pyxisToXYZ(tmpIndex, &tmpCoord);
		stream << tostr(tmpCoord.x()) << " ";
		stream << tostr(tmpCoord.y()) << " ";
		stream << tostr(tmpCoord.z()) << " ";
	}

	stream << "\n";
}

void cmd_test(tcp::iostream& stream)
{
	stream >> tmpNum;
	stream << tostr(++tmpNum) << '\n';
}

void do_CMD_TEXTURE(tcp::iostream& stream)
{
	getline(stream, tmpStr);
	cout << tmpStr << '\n';

	int nTileCode = getTileCode(tmpStr);

	boost::filesystem::path pngfile = cachedir / (tmpStr + ".png");

	if (!exists(pngfile))
	{
		boost::filesystem::path rgbfile = cachedir / (tmpStr + ".rgb");

		tmpIndex = tmpStr;

		// TODO might not need all 128 rows, and could memset just the tail of each row and the last few
		unsigned char buf[3 * 32 * 128];
		memset(buf, 0, 3 * 32 * 128);
		int nTH = tmpIndex.hasVertexChildren() ? 128 : 64;
		int nTX = 0;
		int nTY = 0;

		Cursor cy[3];
		Cursor cx[3];
		PYXIcosIndex startIndex = tmpIndex;
		startIndex.setResolution(startIndex.getResolution() + 3);

		// TODO need to use tile code here
		istringstream sstream(tex_spec[nTileCode]);
		
		int nQuadCount;
		sstream >> nQuadCount;

		while (nQuadCount != 0)
		{
			cx[0] = Cursor(startIndex);
			cx[1] = cx[0]; cx[1].left(); cx[1].left();
			cx[2] = cx[1]; cx[2].left(); cx[2].left();
			int nOff[3] = { 0, 0, 0 };

			while (true)
			{
				sstream >> tmpStr;

				if (tmpStr == "F")
				{
					for (int nTri = 0; nTri != 3; ++nTri) cx[nTri].forward();
				}
				else if (tmpStr == "L")
				{
					for (int nTri = 0; nTri != 3; ++nTri) cx[nTri].left();
				}
				else if (tmpStr == "R")
				{
					for (int nTri = 0; nTri != 3; ++nTri) cx[nTri].right();
				}
				else if (tmpStr == "N")
				{
					// TODO support proper negate
					for (int nTri = 0; nTri != 3; ++nTri) { cx[nTri].left(); cx[nTri].left(); cx[nTri].left(); }
				}
				else
				{
					break;
				}
			}

			for (int nTri = 0; nTri != 3; ++nTri)
			{
				cx[nTri].zoomIn();
				cx[nTri].zoomIn();
				cx[nTri].zoomIn();
				cx[nTri].zoomIn();
				cy[nTri] = cx[nTri];
				for (int nX = 0; nX != 10; ++nX)
				{
					PYXValue v = spCov->getCoverageValue(cx[nTri].getIndex());
					for (int n = 0; n != 3; ++n)
					{
						buf[nTY * 32 * 3 + nTX * 3 + n] = v.getUChar(n);
					}
					cx[nTri].forward();
					++nTX;
				}
			}

			++nTY; nTX = 0;

			for (int nQuad = 0; nQuad != nQuadCount; ++nQuad)
			{
				sstream >> tmpInt;

				for (int nTri = 0; nTri != 3; ++nTri)
				{
					nOff[nTri] += tmpInt;
					switch (tmpInt)
					{
						case -2: cy[nTri].right();
						case -1: cy[nTri].right();
						case 0:  break;
						case 1: cy[nTri].left();
					}
				}

				for (int nY = 0; nY != 9; ++nY)
				{
					for (int nTri = 0; nTri != 3; ++nTri)
					{
						cy[nTri].forward();
						cx[nTri] = cy[nTri];

						switch (nOff[nTri])
						{
							case -2: cx[nTri].left();
							case -1: cx[nTri].left();
							case 0:  break;
							case 1: cx[nTri].right();
						}

						for (int nX = 0; nX != 10; ++nX)
						{
							PYXValue v = spCov->getCoverageValue(cx[nTri].getIndex());
							for (int n = 0; n != 3; ++n)
							{
								buf[nTY * 32 * 3 + nTX * 3 + n] = v.getUChar(n);
							}
							cx[nTri].forward();
							++nTX;
						}
					}
					++nTY; nTX = 0;
				}
			}

			sstream >> nQuadCount;
		}

		if (!tmpIndex.hasVertexChildren())
		{
			// We've already written the pixels past the 64 line texture, so copy them to the right edge.
			int copy[][4] =
			{
				{  0, 63, 30,  9 },
				{  0, 64, 30, 19 },
				{ 10, 63, 30, 29 },
				{ 10, 64, 30, 39 },
				{ 20, 63, 30, 49 },
				{ 20, 64, 30, 59 }
			};

			int sx, sy, dx, dy;

			for (int nE = 0; nE != sizeof(copy) / sizeof(copy[0]); ++nE)
			{
				sx = copy[nE][0];
				sy = copy[nE][1];
				dx = copy[nE][2];
				dy = copy[nE][3];
				for (int ny = 0; ny != 2; ++ny)
				{
					for (int nx = 0; nx != 10; ++nx)
					{
						int si = (sy + ny) * 32 * 3 + (sx + nx) * 3;
						int di = (dy - nx) * 32 * 3 + (dx + ny) * 3;
						buf[di + 0] = buf[si + 0];
						buf[di + 1] = buf[si + 1];
						buf[di + 2] = buf[si + 2];
					}
				}
			}

		}

		ofstream file(rgbfile.string().c_str(), ios::binary);
		file.write((char*)buf, 3 * 32 * nTH);
		file.close();
		std::string cmd = std::string("convert -size 32x") + toString(nTH) + " -depth 8 " + rgbfile.string() + " " + pngfile.string();
		system(cmd.c_str());
		remove(rgbfile);
	}

	ifstream filestream(pngfile.string().c_str(), ios::binary);
	int nSize = file_size(pngfile);
	writeInt(stream, nSize);
	int nCount = 0;
	char c;
	while (filestream.get(c))
	{
		stream.put(c);
		++nCount;
	}
	if (nCount != nSize) cout << "error " << nCount << " " << nSize << "\n";
	stream.flush();
}

// Assumes a tile of depth 2, sends its coords at an additional res
// (so we have centers and vertices of each cell)
void do_CMD_TILECOORD(tcp::iostream& stream)
{
	//stream >> tmpInt;
	tmpInt = 1;

	for (int n = 0; n != tmpInt; ++n)
	{
		getline(stream, tmpStr);
		cout << tmpStr << '\n';
		tmpIndex = tmpStr;

#if WRITE_XYZ
		file.open((cachedir / (tmpStr + ".xyz")).string().c_str(), ios::binary | ios::out);
#endif

		int nCoordCount = 0;

		PYXExhaustiveIterator it(tmpIndex, tmpIndex.getResolution() + 3);
		for (; !it.end(); it.next())
		{
			stream_coord(stream, it.getIndex());
			++nCoordCount;
		}

		if (!tmpIndex.hasVertexChildren())
		{
			// Edge coords of minor tile
			Cursor c(tmpIndex);
			c.zoomIn();
			c.forward();
			c.zoomIn();
			c.zoomIn();
			c.left();
			c.backward();
			for (int n = 0; n != 6; ++n)
			{
				stream_coord(stream, c.getIndex()); ++nCoordCount;
				c.left();
				c.forward();
				stream_coord(stream, c.getIndex()); ++nCoordCount;
				c.right();
				c.forward();
				stream_coord(stream, c.getIndex()); ++nCoordCount;
				c.left();
				c.forward();
			}
		}

		stream.flush();

#if WRITE_XYZ
		file.close();
#endif
	}
}

// TODO needs proper implmentation
void do_CMD_TILESET(tcp::iostream& stream)
{
	cout << '\n';

	PYXIcosIterator it(3);

	for (; !it.end(); it.next())
	{
		const PYXIcosIndex& index = it.getIndex();
		if (!index.isPentagon())
		{
			stream << index.toString() << '\n';
		}
	}
	stream << "\n";
	stream.flush();
}

void cmd_tileset(tcp::iostream& stream)
{
	double d;
	double t;

	for (int n = 0; n != 16; ++n)
	{
		stream >> m[n];
	}
	stream >> d;
	stream >> t;

	int nRes = 10;
	int nHexRes = nRes - 5;
	int nTileRes = nHexRes - 2;

	std::vector<PYXIcosIndex> vec(vecRes2);

	while (!vec.empty())
	{
		PYXIcosIndex& index = vec.back();
		if (index.getResolution() != nTileRes)
		{
			// Classify cells at non-tile resolution.
			int nClass = classify_cell_to_frustrum(index);
			if (nClass == knOutside)
			{
				vec.pop_back();
			}
#if 0
			else if (nClass == knInside)
			{
				pSnyder->pyxisToXYZ(index, &tmpCoord);
				stream << index.toString() << ' '
					<< tostr(tmpCoord.x()).c_str() << ' '
					<< tostr(tmpCoord.y()).c_str() << ' '
					<< tostr(tmpCoord.z()).c_str() << ' ';
				vec.pop_back();
			}
#endif
			else
			{
				PYXChildIterator it(index);
				vec.pop_back();
				for (int n = 0; !it.end(); it.next(), ++n)
				{
					vec.insert(vec.end() - n, it.getIndex());
				}
			}
		}
		else
		{
			// Classify points at tile resolution.
			if (classify_point_to_frustrum(index) == knInside)
			{
				// TEMP only do this for hexagon tiles
				if (!index.isPentagon())
				{
					stream << index.toString() << ' ';
#if 0
					PYXExhaustiveIterator it(index, nHexRes + 1);
					for (; !it.end(); it.next())
					{
						pSnyder->pyxisToXYZ(it.getIndex(), &tmpCoord);
						stream
							<< tostr(tmpCoord.x()).c_str() << ' '
							<< tostr(tmpCoord.y()).c_str() << ' '
							<< tostr(tmpCoord.z()).c_str() << ' ';
					}
					stream << "\n";
#endif
				}
			}
			vec.pop_back();
		}
	}

	stream << "\n";
}

int main(int argc, char* argv[])
{
	PYXLibInstance::initialize("nerve");

	init_nerve();

	cmd_t cmd;

	m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 0;
	m[4] = 0; m[5] = 1; m[6] = 0; m[7] = 0;
	m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
	tmpIndex = "A-02";
	//classify_point_to_frustrum(tmpIndex);

	try
	{
		boost::asio::io_service io_service;

		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 12345));

		tcp::iostream stream;
		acceptor.accept(*stream.rdbuf());

		while (true)
		{
			cmd = stream.get();
			cout << "cmd " << (int)cmd << '\n';

			// TODO could use switch statement now that ints are used for cmd codes
#define CMD(x) \
			else if (cmd == CMD_##x) \
			{ \
				cout << "CMD_" #x " "; \
				do_CMD_##x(stream); \
			}

			if (cmd == CMD_EXIT)
			{
				cout << "exiting\n";
				break;
			}
			//CMD(snyder)
			//CMD(test)
			CMD(TEXTURE)
			CMD(TILECOORD)
			CMD(TILESET)
			else
			{
				cout << "unknown command\n";
				break;
			}
		}
	}
	catch (...)
	{
		cerr << "Unknown exception.\n";
		return 1;
	}

	PYXLibInstance::uninitialize();

	cout << "ended\n";

	return 0;
}
