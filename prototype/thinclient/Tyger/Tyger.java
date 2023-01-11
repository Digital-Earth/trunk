// Tyger by Marc Lepage 2007-05-26

/*
 * Tile codes:
 * 0 - class 1 major
 * 1 - class 1 minor
 * 2 - class 2 major
 * 3 - class 2 minor
*/

import java.applet.Applet;
import java.awt.BorderLayout;
import java.awt.event.*;
import java.awt.GraphicsConfiguration;
import javax.media.j3d.*;
import javax.vecmath.*;
import com.sun.j3d.utils.applet.MainFrame;
import com.sun.j3d.utils.behaviors.mouse.*;
import com.sun.j3d.utils.geometry.*;
import com.sun.j3d.utils.image.TextureLoader;
import com.sun.j3d.utils.universe.*;

import java.io.FileInputStream;
import java.nio.*;
import java.nio.channels.*;

import java.net.*;
import java.io.*;
import java.util.Vector;

public class Tyger extends Applet
{
	private Socket socket;
	private DataInputStream dis;
	private DataOutputStream dos;

	private static final byte CMD_EXIT			= 1;
	private static final byte CMD_TILESET = 10;
	private static final byte CMD_TILECOORD = 11;
	private static final byte CMD_TEXTURE = 12;

	private static final File cachedir = new File("tyger_cache");

	private static final int IOBUFSIZE = 1024 * 16;
	private static final byte[] iobuf = new byte[IOBUFSIZE];

	// The number of coordinates per tile, indexed by tile code.
	private static final int[] coordCount = { 55, 31+12, 55, 31+12 };

	private static final int[] rawCoordCount = { 55, 31, 55, 31 };

	private static final int[][] coordIndices =
	{
		{	// class 1 major tile
			 2,  0,  1,  6,  7, 51, 18, 50,
			 4,  0,  3,  2,  9, 25, 28, 24,
			 6,  0,  5,  4, 11, 35, 44, 40,
			 7, 18,  1, 17,  2,  8, 25, 26, 24, 20, 23, 22,
			 9, 28,  3, 33,  4, 10, 35, 36, 40, 34, 39, 38,
			11, 44,  5, 43,  6, 12, 51, 52, 50, 48, 49, 54,
			26, 8,  16, 17, 13, 18, 14, 19,
			36, 10, 32, 33, 27, 28, 30, 29,
			52, 12, 42, 43, 41, 44, 46, 45,
			16, 13, 15, 14,
			32, 27, 31, 30,
			42, 41, 47, 46,
			20, 26, 22, 21,
			34, 36, 38, 37,
			48, 52, 54, 53
		},
		{	// class 1 minor tile
			 2,  0,  1,  6,  7, 29, 13, 30,
			 4,  0,  3,  2,  9, 17, 19, 18,
			 6,  0,  5,  4, 11, 23, 25, 24,
			 7, 13,  1, 14,  2,  8, 17, 16,
			 9, 19,  3, 20,  4, 10, 23, 22,
			11, 25,  5, 26,  6, 12, 29, 28,
			 8, 14, 16, 15,
			10, 20, 22, 21,
			12, 26, 28, 27
		},
		{	// class 2 major tile
			 0,  1,  3,  2, 33,  8, 28, 24,
			 0,  3,  5,  4, 43, 10, 44, 40,
			 0,  5,  1,  6, 17, 12, 18, 50,
			24,  8, 25,  2,  7,  1, 16, 17, 13, 18, 14, 19,
			40, 10, 35,  4,  9,  3, 32, 33, 27, 28, 30, 29,
			50, 12, 51,  6, 11,  5, 42, 43, 41, 44, 46, 45,
			 7, 16, 25, 26, 24, 20, 23, 22,
			 9, 32, 35, 36, 40, 34, 39, 38,
			11, 42, 51, 52, 50, 48, 49, 54,
			20, 26, 22, 21,
			34, 36, 38, 37,
			48, 52, 54, 53,
			16, 13, 15, 14,
			32, 27, 31, 30,
			42, 41, 47, 46
		},
		{	// class 2 minor tile
			 0,  1,  3,  2, 20,  8, 19, 18,
			 0,  3,  5,  4, 26, 10, 25, 24,
			 0,  5,  1,  6, 14, 12, 13, 30,
			18,  8, 17,  2,  7,  1, 15, 14,
			24, 10, 23,  4,  9,  3, 21, 20,
			30, 12, 29,  6, 11,  5, 27, 26,
			17,  7, 31, 33,
			23,  9, 35, 37,
			29, 11, 39, 41, // end of actual coordinates
			31, 33, 32, 34,
			32, 34, 16, 15,
			35, 37, 36, 38,
			36, 38, 22, 21,
			39, 41, 40, 42,
			40, 42, 28, 27
		}
	};

	// used for interpolating coordinates for minor tiles
	// ordered by class less 1
	private static final int[][][] interpIndices =
	{
		{ { 8, 16 }, { 14, 15 }, { 10, 22 }, { 20, 21 }, { 12, 28 }, { 26, 27 } },
		{ { 17, 16 }, { 7, 15 }, { 23, 22 }, { 9, 21 }, { 29, 28 }, { 11, 27 } }
	};

	// TODO this may be able to be halved (but remember pentagons)
	private static final int[][] stripIndexCounts =
	{
		{ 8, 8, 8, 12, 12, 12, 8, 8, 8, 4, 4, 4, 4, 4, 4 },
		{ 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4 },
		{ 8, 8, 8, 12, 12, 12, 8, 8, 8, 4, 4, 4, 4, 4, 4 },
		{ 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4 }
	};

	private static final int[] texCoordSetMap = { 0 };

	private static final int[][] texCoordRaw =
	{
		{ 3, 5, 3, 1, 1	},
		{ 3, 3, 1 }
	};

	static
	{
		int nSq = 10;

		for (int nSet = 0; nSet != texCoordRaw.length; ++nSet)
		{
			for (int nQ = 0; nQ != texCoordRaw[nSet].length; ++nQ)
			{

			}
		}
	}

	private static float[][] texCoords =
	{
		{	// major tile
			0.296875f, 0.99609375f,
			0.015625f, 0.99609375f,
			0.296875f, 0.92578125f,
			0.015625f, 0.92578125f,
			0.296875f, 0.85546875f,
			0.015625f, 0.85546875f,
			0.296875f, 0.78515625f,
			0.015625f, 0.78515625f,
			0.609375f, 0.99609375f,
			0.328125f, 0.99609375f,
			0.609375f, 0.92578125f,
			0.328125f, 0.92578125f,
			0.609375f, 0.85546875f,
			0.328125f, 0.85546875f,
			0.609375f, 0.78515625f,
			0.328125f, 0.78515625f,
			0.921875f, 0.99609375f,
			0.640625f, 0.99609375f,
			0.921875f, 0.92578125f,
			0.640625f, 0.92578125f,
			0.921875f, 0.85546875f,
			0.640625f, 0.85546875f,
			0.921875f, 0.78515625f,
			0.640625f, 0.78515625f,
			0.296875f, 0.77734375f,
			0.015625f, 0.77734375f,
			0.296875f, 0.70703125f,
			0.015625f, 0.70703125f,
			0.296875f, 0.63671875f,
			0.015625f, 0.63671875f,
			0.296875f, 0.56640625f,
			0.015625f, 0.56640625f,
			0.296875f, 0.49609375f,
			0.015625f, 0.49609375f,
			0.296875f, 0.42578125f,
			0.015625f, 0.42578125f,
			0.609375f, 0.77734375f,
			0.328125f, 0.77734375f,
			0.609375f, 0.70703125f,
			0.328125f, 0.70703125f,
			0.609375f, 0.63671875f,
			0.328125f, 0.63671875f,
			0.609375f, 0.56640625f,
			0.328125f, 0.56640625f,
			0.609375f, 0.49609375f,
			0.328125f, 0.49609375f,
			0.609375f, 0.42578125f,
			0.328125f, 0.42578125f,
			0.921875f, 0.77734375f,
			0.640625f, 0.77734375f,
			0.921875f, 0.70703125f,
			0.640625f, 0.70703125f,
			0.921875f, 0.63671875f,
			0.640625f, 0.63671875f,
			0.921875f, 0.56640625f,
			0.640625f, 0.56640625f,
			0.921875f, 0.49609375f,
			0.640625f, 0.49609375f,
			0.921875f, 0.42578125f,
			0.640625f, 0.42578125f,
			0.296875f, 0.41796875f,
			0.015625f, 0.41796875f,
			0.296875f, 0.34765625f,
			0.015625f, 0.34765625f,
			0.296875f, 0.27734375f,
			0.015625f, 0.27734375f,
			0.296875f, 0.20703125f,
			0.015625f, 0.20703125f,
			0.609375f, 0.41796875f,
			0.328125f, 0.41796875f,
			0.609375f, 0.34765625f,
			0.328125f, 0.34765625f,
			0.609375f, 0.27734375f,
			0.328125f, 0.27734375f,
			0.609375f, 0.20703125f,
			0.328125f, 0.20703125f,
			0.921875f, 0.41796875f,
			0.640625f, 0.41796875f,
			0.921875f, 0.34765625f,
			0.640625f, 0.34765625f,
			0.921875f, 0.27734375f,
			0.640625f, 0.27734375f,
			0.921875f, 0.20703125f,
			0.640625f, 0.20703125f,
			0.296875f, 0.19921875f,
			0.015625f, 0.19921875f,
			0.296875f, 0.12890625f,
			0.015625f, 0.12890625f,
			0.609375f, 0.19921875f,
			0.328125f, 0.19921875f,
			0.609375f, 0.12890625f,
			0.328125f, 0.12890625f,
			0.921875f, 0.19921875f,
			0.640625f, 0.19921875f,
			0.921875f, 0.12890625f,
			0.640625f, 0.12890625f,
			0.296875f, 0.12109375f,
			0.015625f, 0.12109375f,
			0.296875f, 0.05078125f,
			0.015625f, 0.05078125f,
			0.609375f, 0.12109375f,
			0.328125f, 0.12109375f,
			0.609375f, 0.05078125f,
			0.328125f, 0.05078125f,
			0.921875f, 0.12109375f,
			0.640625f, 0.12109375f,
			0.921875f, 0.05078125f,
			0.640625f, 0.05078125f
		},
		{	// minor tile
			0.296875f, 0.9921875f,
			0.015625f, 0.9921875f,
			0.296875f, 0.8515625f,
			0.015625f, 0.8515625f,
			0.296875f, 0.7109375f,
			0.015625f, 0.7109375f,
			0.296875f, 0.5703125f,
			0.015625f, 0.5703125f,
			0.609375f, 0.9921875f,
			0.328125f, 0.9921875f,
			0.609375f, 0.8515625f,
			0.328125f, 0.8515625f,
			0.609375f, 0.7109375f,
			0.328125f, 0.7109375f,
			0.609375f, 0.5703125f,
			0.328125f, 0.5703125f,
			0.921875f, 0.9921875f,
			0.640625f, 0.9921875f,
			0.921875f, 0.8515625f,
			0.640625f, 0.8515625f,
			0.921875f, 0.7109375f,
			0.640625f, 0.7109375f,
			0.921875f, 0.5703125f,
			0.640625f, 0.5703125f,
			0.296875f, 0.5546875f,
			0.015625f, 0.5546875f,
			0.296875f, 0.4140625f,
			0.015625f, 0.4140625f,
			0.296875f, 0.2734375f,
			0.015625f, 0.2734375f,
			0.296875f, 0.1328125f,
			0.015625f, 0.1328125f,
			0.609375f, 0.5546875f,
			0.328125f, 0.5546875f,
			0.609375f, 0.4140625f,
			0.328125f, 0.4140625f,
			0.609375f, 0.2734375f,
			0.328125f, 0.2734375f,
			0.609375f, 0.1328125f,
			0.328125f, 0.1328125f,
			0.921875f, 0.5546875f,
			0.640625f, 0.5546875f,
			0.921875f, 0.4140625f,
			0.640625f, 0.4140625f,
			0.921875f, 0.2734375f,
			0.640625f, 0.2734375f,
			0.921875f, 0.1328125f,
			0.640625f, 0.1328125f,
			0.296875f, 0.1171875f,
			0.015625f, 0.1171875f,
			0.296875f, 0.5f/64f,
			0.015625f, 0.5f/64f,
			0.609375f, 0.1171875f,
			0.328125f, 0.1171875f,
			0.609375f, 0.5f/64f,
			0.328125f, 0.5f/64f,
			0.921875f, 0.1171875f,
			0.640625f, 0.1171875f,
			0.921875f, 0.5f/64f,
			0.640625f, 0.5f/64f, // end of regular tex coords

			30.5f/32f, 63.5f/64f,
			30.5f/32f, 54.5f/64f,
			31.5f/32f, 63.5f/64f,
			31.5f/32f, 54.5f/64f,
			30.5f/32f, 53.5f/64f,
			30.5f/32f, 44.5f/64f,
			31.5f/32f, 53.5f/64f,
			31.5f/32f, 44.5f/64f,

			30.5f/32f, 43.5f/64f,
			30.5f/32f, 34.5f/64f,
			31.5f/32f, 43.5f/64f,
			31.5f/32f, 34.5f/64f,
			30.5f/32f, 33.5f/64f,
			30.5f/32f, 24.5f/64f,
			31.5f/32f, 33.5f/64f,
			31.5f/32f, 24.5f/64f,

			30.5f/32f, 23.5f/64f,
			30.5f/32f, 14.5f/64f,
			31.5f/32f, 23.5f/64f,
			31.5f/32f, 14.5f/64f,
			30.5f/32f, 13.5f/64f,
			30.5f/32f,  4.5f/64f,
			31.5f/32f, 13.5f/64f,
			31.5f/32f,  4.5f/64f
		}
	};

	private static final double[] testCoords =
	{
		0.0, 3.0, 0, // 1
		0.0, 2.0, 0, // 3
		1.0, 2.0, 0, // 2
		0.0, 0.0, 0, // 7
		1.0, 1.0, 0, // 4
		1.0, 3.0, 0, // 0
		1.0, 0.0, 0, // 6
		0.0, 1.0, 0, // 5
	};

	private static final float[] testTexCoords =
	{
		0.296875f, 0.99609375f,
		0.015625f, 0.99609375f,
		0.296875f, 0.92578125f,
		0.015625f, 0.92578125f,
		0.296875f, 0.85546875f,
		0.015625f, 0.85546875f,
		0.296875f, 0.78515625f,
		0.015625f, 0.78515625f
	};

	private static final int[] testIndices = { 0, 1, 3, 2, 33, 8, 28, 24 };
	private static final int[] testTexIndices = { 5, 0, 2, 1, 4, 7, 6, 3 };

	private static final int[] testStripCounts = { 8 };

	private static final int[][] texCoordIndices = new int[texCoords.length][];
	static
	{
		for (int n = 0; n != texCoordIndices.length; ++n)
		{
			texCoordIndices[n] = new int[texCoords[n].length / 2];
			for (int n2 = 0; n2 != texCoordIndices[n].length; ++n2)
			{
				texCoordIndices[n][n2] = n2;
			}
		}
	}

	//static
	//{
	//    for (int n = 0; n != texCoords[0].length; n += 4)
	//    {
	//        float s = texCoords[0][n + 0];
	//        float t = texCoords[0][n + 1];
	//        texCoords[0][n + 0] = texCoords[0][n + 2];
	//        texCoords[0][n + 1] = texCoords[0][n + 3];
	//        texCoords[0][n + 2] = s;
	//        texCoords[0][n + 3] = t;
	//    }
	//}
	//static
	//{
	//    for (int n = 1; n < texCoords[0].length; n += 2)
	//    {
	//        texCoords[0][n] = 1 - texCoords[0][n];
	//    }
	//}

	private static final double[] coordinates =
	{
		0.0696125, -0.349966, 0.934172, 0.114013, -0.342149, 0.932703, 0.0831634, -0.309029, 0.94741, 0.0414273, -0.317331, 0.94741, 0.0256002, -0.359735, 0.932703, 0.0531845, -0.38935, 0.919553, 0.0998616, -0.380065, 0.919553, 0.123394, -0.304456, 0.9445, 0.0547032, -0.275012, 0.959883, 0.00250886, -0.328502, 0.9445, 0.00829597, -0.399348, 0.916762, 0.0824049, -0.414278, 0.906412, 0.14516, -0.372125, 0.916762, 0.202118, -0.324425, 0.924065, 0.245686, -0.31454, 0.916899, 0.212936, -0.285435, 0.934444, 0.168738, -0.295021, 0.940473, 0.158205, -0.333632, 0.929334, 0.189549, -0.363185, 0.912232, 0.233436, -0.353516, 0.905833, 0.109757, -0.225267, 0.968095, 0.150819, -0.218747, 0.964056, 0.122758, -0.182566, 0.9755, 0.081204, -0.190081, 0.978404, 0.0680389, -0.232733, 0.970158, 0.0965518, -0.267442, 0.958725, 0.137531, -0.261518, 0.95535, -0.0151963, -0.250121, 0.968095, 0.0262035, -0.241055, 0.970158, -0.00228179, -0.206687, 0.978404, -0.0435483, -0.215646, 0.9755, -0.0556277, -0.259811, 0.964056, -0.026984, -0.294242, 0.95535, 0.0131433, -0.284033, 0.958725, -0.0625808, -0.377077, 0.924065, -0.0184867, -0.368778, 0.929334, -0.0429941, -0.337137, 0.940473, -0.0874961, -0.345195, 0.934444, -0.106615, -0.384617, 0.916899, -0.0803823, -0.415939, 0.905833, -0.0361353, -0.408077, 0.912232, 0.0200036, -0.465739, 0.884696, 0.0657379, -0.453871, 0.888639, 0.0366399, -0.427961, 0.903054, -0.00848537, -0.437762, 0.899051, -0.0252046, -0.475177, 0.879529, 0.00330095, -0.502621, 0.8645, 0.0489491, -0.491681, 0.869398, 0.159749, -0.437941, 0.884696, 0.205128, -0.429361, 0.879529, 0.175364, -0.401192, 0.899051, 0.129923, -0.409406, 0.903054, 0.112955, -0.444479, 0.888639, 0.142935, -0.472986, 0.869398, 0.189295, -0.465625, 0.8645
	};

	private boolean isTileMajor(String name)
	{
		return name.charAt(name.length() - 1) == '0';
	}

	private int getTileClass(String name)
	{
		return 2 - (name.length() - (name.charAt(2) == '-' ? 1 : 0)) % 2;
	}

	private int getTileCode(String name)
	{
		boolean bMajor = isTileMajor(name);
		int nClass = getTileClass(name);
		return (nClass - 1) * 2 + (bMajor ? 0 : 1);
	}

	public Shape3D createTile(String name)
	{
		int nTileCode = getTileCode(name);
		int nTexCode = isTileMajor(name) ? 0 : 1;

		try
		{
			System.out.println("CMD_TILECOORD " + name);
			dos.writeByte(CMD_TILECOORD);
			dos.writeBytes(name);
			dos.writeByte('\n');

			int nRawCoordCount = rawCoordCount[nTileCode];
			int nCoordCount = coordCount[nTileCode];
			double[] coords = new double[nCoordCount * 3];
			for (int n = 0; n != nRawCoordCount * 3; ++n)
			{
				coords[n] = dis.readDouble();
			}

			if (!isTileMajor(name))
			{
				int nK = nRawCoordCount;
				int[][] extra = interpIndices[getTileClass(name) - 1];
				for (int nE = 0; nE != extra.length; ++nE)
				{
					for (int n = 0; n != 3; ++n)
					{
						coords[nK*3 + n] = coords[extra[nE][0]*3 + n]*(2/9.0) + coords[extra[nE][1]*3 + n]*(7/9.0);
					}
					++nK;
					for (int n = 0; n != 3; ++n)
					{
						coords[nK*3 + n] = coords[extra[nE][0]*3 + n]*(1/9.0) + coords[extra[nE][1]*3 + n]*(8/9.0);
					}
					++nK;
				}
			}

			File texfile = new File(cachedir, name + ".png");

			if (!texfile.exists())
			{
				System.out.println("CMD_TEXTURE " + name);
				dos.writeByte(CMD_TEXTURE);
				dos.writeBytes(name);
				dos.writeByte('\n');

				FileOutputStream fos = new FileOutputStream(texfile);

				int nSize = dis.readInt();
				int nCount = 0;

				while (true)
				{
					int nToRead = Math.min(iobuf.length, nSize - nCount);
					int nRead = dis.read(iobuf, 0, nToRead);
					if (nRead == -1)
					{
						break;
					}
					nCount += nRead;
					fos.write(iobuf, 0, nRead);
					if (nCount == nSize)
					{
						break;
					}
				}

				fos.close();
			}

			TextureLoader texldr = new TextureLoader(texfile.getPath(), new String("RGB"), this);
			Texture tex = texldr.getTexture();

			// USE_COORD_INDEX_ONLY will only use the coordinate index (for coords, texcoords, normals, etc.
			// Can't set BY_REFERENCE_INDICES if USE_COORD_INDEX is not set

			IndexedTriangleStripArray itsa = new IndexedTriangleStripArray(
				coords.length / 3,
				GeometryArray.COORDINATES | GeometryArray.TEXTURE_COORDINATE_2
					/*| GeometryArray.USE_COORD_INDEX_ONLY*/ | GeometryArray.BY_REFERENCE /*| GeometryArray.BY_REFERENCE_INDICES*/,
					//| GeometryArray.USE_NIO_BUFFER,
				//texCoordSetMap.length,
				//texCoordSetMap,
				coordIndices[nTileCode].length,
				stripIndexCounts[nTileCode]);

			itsa.setCoordRefDouble(coords);
			//itsa.setCoordRefBuffer(coordBuf);

			itsa.setCoordinateIndices(0, coordIndices[nTileCode]);//itsa.setCoordIndicesRef(coordIndices[nTileCode]);

			itsa.setTexCoordRefFloat(0, texCoords[nTexCode]);
			itsa.setTextureCoordinateIndices(0, 0, texCoordIndices[nTexCode]);

			PolygonAttributes pa = new PolygonAttributes();
			pa.setPolygonMode(PolygonAttributes.POLYGON_LINE);

			Appearance app = new Appearance();
			//app.setPolygonAttributes(pa);
			app.setTexture(tex);

			return new Shape3D(itsa, app);
		}
		catch (Exception e)
		{
			System.out.println(e.getMessage());
			System.exit(1);
		}

		return null;
	}

	Vector getTileSet() throws Exception
	{
		System.out.println("CMD_TILESET");
		dos.writeByte(CMD_TILESET);

		Vector vec = new Vector();

		while (true)
		{
			String str = dis.readLine();
			if (str.length() == 0)
			{
				break;
			}
			vec.addElement(str);
		}

		return vec;
	}

	//public Shape3D createTestStrip()
	//{
	//    double[] coords = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0 }
	//    int[] indices = { 0, 1, 2, 3 };
	//    int[] strips = { 4 };
	//    return new Shape3D(
	//        new IndexedTriangleStripArray(
	//            coords.length / 2,
	//            indices),
	//        new Appearance());
	//}

	public BranchGroup createSceneGraph() throws Exception
	{
		// Create the root of the branch graph
		BranchGroup objRoot = new BranchGroup();

		// Create the transform group node and initialize it to the
		// identity.  Enable the TRANSFORM_WRITE capability so that
		// our behavior code can modify it at runtime.  Add it to the
		// root of the subgraph.
		TransformGroup objTrans = new TransformGroup();
		objTrans.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
		//objRoot.addChild(objTrans);

		// Create a simple shape leaf node, add it to the scene graph.
		Sphere sph = new Sphere(0.25f, new Appearance());
		//objTrans.addChild(sph);
		//objTrans.addChild(createTile());

		// Create the bounding leaf node
		BoundingSphere bounds =
			   new BoundingSphere(new Point3d(0.0, 0.0, 0.0), 100.0);
		BoundingLeaf boundingLeaf = new BoundingLeaf(bounds);
		objTrans.addChild(boundingLeaf);

		Vector vec = getTileSet();

		// Create the transform node
		TransformGroup transformGroup = new TransformGroup();
		transformGroup.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);
		transformGroup.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
		//transformGroup.addChild(sph);

		java.util.Iterator it = vec.iterator();

		while (it.hasNext())
		{
			transformGroup.addChild(createTile((String)it.next()));
		}

		objRoot.addChild(transformGroup);

		// Create the drag behavior node
		MouseRotate behavior = new MouseRotate();
		behavior.setTransformGroup(transformGroup);
		transformGroup.addChild(behavior);
		behavior.setSchedulingBounds(bounds);

		// Create the zoom behavior node
		MouseZoom behavior2 = new MouseZoom();
		behavior2.setTransformGroup(transformGroup);
		transformGroup.addChild(behavior2);
		behavior2.setSchedulingBounds(bounds);

		// Create the zoom behavior node
		MouseTranslate behavior3 = new MouseTranslate();
		behavior3.setTransformGroup(transformGroup);
		transformGroup.addChild(behavior3);
		behavior3.setSchedulingBounds(bounds);

		// Have Java 3D perform optimizations on this scene graph.
		objRoot.compile();

		return objRoot;
	}

	public Tyger()
	{
		try
		{
			if (!cachedir.exists())
			{
				cachedir.mkdir();
			}

			socket = new Socket("localhost", 12345);
			dis = new DataInputStream(socket.getInputStream());
			dos = new DataOutputStream(socket.getOutputStream());

			setLayout(new BorderLayout());
			GraphicsConfiguration config =
			   SimpleUniverse.getPreferredConfiguration();

			Canvas3D c = new Canvas3D(config);
			add("Center", c);

			// Create a simple scene and attach it to the virtual universe
			BranchGroup scene = createSceneGraph();
			SimpleUniverse u = new SimpleUniverse(c);

			// This will move the ViewPlatform back a bit so the
			// objects in the scene can be viewed.
			u.getViewingPlatform().setNominalViewingTransform();

			u.addBranchGraph(scene);
		}
		catch (Exception e)
		{
			System.out.println(e);
			System.exit(1);
		}
	}

	//
	// The following allows the applet to be run as an application
	//
	public static void main(String[] args)
	{
		new MainFrame(new Tyger(), 1280, 800);
	}
}
