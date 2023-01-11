using System;
using System.IO;
using System.Net;

namespace Pyxis.Utilities
{
    /// <summary>
    /// RemoteStream allow to perform random access to Http resources from servers that supply Accept-Range: bytes.
    /// 
    /// RemoteStream would read chunks of 8Kb using Range header based on the Read/Seek operations.
    /// </summary>
    public class RemoteStream : Stream
    {
        private readonly Uri m_target;
        private long m_length;

        private const int BufferMaxSize = 1024 * 8;

        class Buffer
        {
            private readonly byte[] m_buffer;
            private readonly long m_bufferStart;
            private readonly int m_bufferSize;

            public Buffer(long startPosition, int size, Stream stream)
            {
                m_bufferSize = size;
                m_bufferStart = startPosition;
                m_buffer = new byte[size];

                var offset = 0;
                while (offset < m_bufferSize)
                {
                    var read = stream.Read(m_buffer, offset, m_bufferSize - offset);
                    offset += read;
                }
            }

            public bool HasPosition(long position)
            {
                return position >= m_bufferStart && position < m_bufferStart + m_bufferSize;
            }

            public int Read(long position, byte[] buffer, int offset, int count)
            {
                int bufferOffset = (int)(position - m_bufferStart);
                int read = Math.Min(count, m_bufferSize - bufferOffset);

                Array.Copy(m_buffer, bufferOffset, buffer, offset, read);

                return read;
            }
        }

        private Buffer m_currentBuffer = null;
        private Buffer m_oldBuffer = null;

        public RemoteStream(Uri target)
        {
            m_target = target;

            InitStream();
        }

        private void InitStream()
        {
            var webRequest = WebRequest.Create(m_target);

            var httpWebRequest = webRequest as HttpWebRequest;

            if (httpWebRequest != null)
            {
                GetHttpContentSize(httpWebRequest);
            }

            var ftpWebRequest = webRequest as FtpWebRequest;

            if (ftpWebRequest != null)
            {
                ftpWebRequest.Method = WebRequestMethods.Ftp.GetFileSize;

                using (var response = webRequest.GetResponse())
                {
                    m_length = response.ContentLength;
                }
            }
        }

        private void GetHttpContentSize(HttpWebRequest httpWebRequest)
        {
            httpWebRequest.Method = "HEAD";
            httpWebRequest.Headers["UserAgent"] = "Pyxis Crawler";
            httpWebRequest.Accept = "*/*";
            httpWebRequest.Headers["AcceptEncoding"] = "gzip, deflate";
            httpWebRequest.Timeout = (int)TimeSpan.FromSeconds(20).TotalMilliseconds;

            using (var response = httpWebRequest.GetResponse())
            {
                m_length = response.ContentLength;
                if (response.Headers["Accept-Ranges"] != "bytes")
                {
                    throw new Exception("server doesn't allow range requests");
                }
            }
        }

        private void ReadBuffer(long position)
        {
            if (m_currentBuffer != null && m_currentBuffer.HasPosition(position))
            {
                return;
            }

            if (m_oldBuffer != null && m_oldBuffer.HasPosition(position))
            {
                //swap buffers
                var temp = m_currentBuffer;
                m_currentBuffer = m_oldBuffer;
                m_oldBuffer = temp;
                return;
            }

            var bufferStart = position - (position % BufferMaxSize);
            var bufferSize = (int)Math.Min(m_length - bufferStart, BufferMaxSize);
            
            Console.WriteLine("Read " + position + " " + bufferSize);

            var webRequest = WebRequest.Create(m_target);

            var httpWebRequest = webRequest as HttpWebRequest;

            if (httpWebRequest != null)
            {
                httpWebRequest.Headers["UserAgent"] = "Pyxis Crawler";            
                httpWebRequest.Accept = "*/*";
                httpWebRequest.Headers["AcceptEncoding"] = "gzip, deflate";
                httpWebRequest.AddRange("bytes", bufferStart, bufferStart + bufferSize - 1);
                httpWebRequest.Timeout = (int)TimeSpan.FromSeconds(20).TotalMilliseconds;

                using (var response = httpWebRequest.GetResponse())
                {
                    m_oldBuffer = m_currentBuffer;
                    m_currentBuffer = new Buffer(bufferStart, bufferSize, response.GetResponseStream());
                }
            }

            var ftpWebRequest = webRequest as FtpWebRequest;
            if (ftpWebRequest != null)
            {
                ftpWebRequest.ContentOffset = bufferStart;

                using (var response = webRequest.GetResponse())
                {
                    m_oldBuffer = m_currentBuffer;
                    m_currentBuffer = new Buffer(bufferStart, bufferSize, response.GetResponseStream());
                }
            }
        }

        public override void Flush()
        {
            return;
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            switch (origin)
            {
                case SeekOrigin.Begin:
                    Position = offset;
                    break;
                case SeekOrigin.Current:
                    Position += offset;
                    break;
                case SeekOrigin.End:
                    Position = Length + offset;
                    break;
            }
            return Position;
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException();
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            ReadBuffer(Position);
            var read = m_currentBuffer.Read(Position, buffer, offset, count);
            Position += read;
            return read;
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException();
        }

        public override bool CanRead
        {
            get { return true; }
        }

        public override bool CanSeek
        {
            get { return true; }
        }

        public override bool CanWrite
        {
            get { return false; }
        }

        public override long Length
        {
            get { return m_length; }
        }

        public override long Position { get; set; }
    }
}
