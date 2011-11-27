using System;

namespace EVEMon.Common.IgbService
{
    public class IgbClientDataReadEventArgs : EventArgs
    {
        private byte[] m_buffer;
        private int m_offset;
        private int m_count;

        public IgbClientDataReadEventArgs(byte[] buffer, int offset, int count)
        {
            m_buffer = buffer;
            m_offset = offset;
            m_count = count;
        }

        public byte[] Buffer
        {
            get { return m_buffer; }
        }

        public int Offset
        {
            get { return m_offset; }
        }

        public int Count
        {
            get { return m_count; }
        }
    }
}