using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Pyxis.Utilities;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Compressed Float 32 Rhombus is a way to send float32 textures by quantizing the values and gzip compressing them.
    /// 
    /// The compression is done in 2 steps:
    /// 1) create lookup table (WantedBinCount defines the number of entries &lt; 255) of float values. we do so by sorting normalizing the data.
    /// 2) quantize each float into 2 bytes: 
    ///      a) first byte (stored in valueBin array) defines the bin to use in the lookup table.
    ///      b) second byte (stored in valueBinOffset array) defines the offset of the current value inside the bin.
    ///      
    /// To restore the value, the reader should do the following: resultValue = bin[binIndex] + (bin[binIndex+1]-bin[binIndex]) * offset / OffsertResultion.
    /// Therefore, the number of possible values is WantedBinCount * BinOffsetResulation which is distributed nonlinearly on the given input values range.
    /// 
    /// Some rhombus can contain null values. a null value is treated as bin=0 offset=0. Aka, bin numbers start from 1.
    /// 
    /// They get stored into a buffer in the following way:
    /// 
    /// Section                   Length (bytes)         Description
    /// -------                   --------------         -----------
    /// MagicHeader               4                      "PYX0"
    /// RhombusSize               4                      size of image (as int)
    /// BinCount                  4                      number of bins (as int)
    /// BinOffsetResolution       4                      offset resolution range (as int)
    /// Bins                      4 * binCount           bins (as floats)
    /// ValueBin                  size * size            byte for each pixel
    /// ValueBinOffset            size * size            byte for each pixel
    /// </summary>
    public class CompressedFloat32Rhombus
    {
        private static readonly byte[] s_magicHeader = Encoding.UTF8.GetBytes("PYX0");

        /// <summary>
        /// Number of nonlinear stops (bins) to generate to compress the float values
        /// </summary>
        private const byte WantedBinCount = 32;

        /// <summary>
        /// Number of values to use to define the offset of a value inside a bin.
        /// </summary>
        private const byte BinOffsetResulation = 255;

        /// <summary>
        /// Create a compress float 32 rhombus
        /// </summary>
        /// <param name="size">size of the rhombus</param>
        /// <param name="hasValuesBlockSize">size in bytes of the hasValuesBlockSize</param>
        /// <param name="buffer">buffer of float32 return from RhombusUtils</param>
        /// <returns>CompressedFloat32Rhombus object</returns>
        public static CompressedFloat32Rhombus Create(int size, int hasValuesBlockSize, byte[] buffer)
        {
            var numberOfPixels = size*size;
            var floats = new float[numberOfPixels];
            var validFloats = new List<float>();
            for (var index = 0; index < numberOfPixels; index++)
            {

                if (buffer[index] == 0)
                {
                    continue;
                }

                floats[index] = BitConverter.ToSingle(buffer, hasValuesBlockSize + index*sizeof (float));
                validFloats.Add(floats[index]);
            }
            validFloats.Sort();

            var bins = CreateBins(validFloats).ToArray();

            var valueBin = new byte[numberOfPixels];
            var valueBinOffset = new byte[numberOfPixels];

            for (var index = 0; index < numberOfPixels; index++)
            {
                if (buffer[index] == 0)
                {
                    valueBin[index] = 0;
                    valueBinOffset[index] = 0;
                }
                else
                {
                    var value = floats[index];
                    var bin = Array.BinarySearch(bins, value);
                    if (bin < 0)
                    {
                        //value not found - we need the ValueBinOffset
                        bin = ~bin;

                        valueBin[index] = (byte) (bin);
                        valueBinOffset[index] = (byte) (Math.Floor((value - bins[bin - 1])/(bins[bin] - bins[bin - 1])*BinOffsetResulation));
                    }
                    else
                    {
                        //value found
                        valueBin[index] = (byte) (bin + 1);
                        valueBinOffset[index] = 0;
                    }
                }
            }

            var rawData = WriteIntoBuffer(size, bins, valueBin, valueBinOffset);

            var result = new CompressedFloat32Rhombus
            {
                Bins = bins,
                GzippedData = Compression.GZip.Compress(rawData)
            };

            return result;
        }

        private static byte[] WriteIntoBuffer(int size, float[] bins, byte[] valueBin, byte[] valueBinOffset)
        {
            var headerSize = s_magicHeader.Length;

            var rhombusSizeBuffer = BitConverter.GetBytes(size);
            var rhombusSizeBufferSize = rhombusSizeBuffer.Length;

            var binCountBuffer = BitConverter.GetBytes(bins.Length);
            var binCountSize = binCountBuffer.Length;

            var binOffsetResolutionBuffer = BitConverter.GetBytes((int) BinOffsetResulation);
            var binOffsetResolutionBufferSize = binOffsetResolutionBuffer.Length;

            var binsSize = sizeof (float)*bins.Length;
            var valueBinSize = sizeof (byte)*valueBin.Length;
            var valueBinOffsetSize = sizeof (byte)*valueBinOffset.Length;

            var result = new byte[headerSize + rhombusSizeBufferSize + binCountSize + binOffsetResolutionBufferSize + binsSize + valueBinSize + valueBinOffsetSize];

            var offset = 0;

            Buffer.BlockCopy(s_magicHeader, 0, result, offset, headerSize);
            offset += headerSize;
            
            Buffer.BlockCopy(rhombusSizeBuffer, 0, result, offset, rhombusSizeBufferSize);
            offset += rhombusSizeBufferSize;
            
            Buffer.BlockCopy(binCountBuffer, 0, result, offset, binCountSize);
            offset += binCountSize;
            
            Buffer.BlockCopy(binOffsetResolutionBuffer, 0, result, offset, binOffsetResolutionBufferSize);
            offset += binOffsetResolutionBufferSize;
            
            Buffer.BlockCopy(bins, 0, result, offset, binsSize);
            offset += binsSize;
            
            Buffer.BlockCopy(valueBin, 0, result, offset, valueBinSize);
            offset += valueBinSize;

            Buffer.BlockCopy(valueBinOffset, 0, result, offset, valueBinOffsetSize);

            return result;
        }

        private static List<float> CreateBins(List<float> validFloats)
        {
            var bins = new List<float>();

            if (validFloats.Count > 0)
            {
                bins.Add(validFloats[0]);

                for (var i = 1; i < WantedBinCount - 1; i++)
                {
                    var newBin = validFloats[validFloats.Count*i/(WantedBinCount - 1)];
                    if (newBin != bins.Last())
                    {
                        bins.Add(newBin);
                    }
                }
                if (bins.Last() != validFloats[validFloats.Count - 1])
                {
                    bins.Add(validFloats[validFloats.Count - 1]);
                }
            }
            return bins;
        }

        /// <summary>
        /// List of float bins been used to compress float32 data
        /// </summary>
        public float[] Bins { get; set; }

        /// <summary>
        /// GzippedData generated for the rhombus data
        /// </summary>
        public byte[] GzippedData { get; private set; }
    }
}
