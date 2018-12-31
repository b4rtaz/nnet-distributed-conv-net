using System;
using System.Net.Sockets;

namespace nNet.mjpeg
{
    public class SocketUtils
    {
        public static void ReadBytes(Socket s, byte[] bytes, int size)
        {
            int read = 0;
            while (read < size)
                read += s.Receive(bytes, read, size - read, SocketFlags.None);
        }

        public static int ReadInt(Socket s)
        {
            var bytes = new byte[sizeof(int)];
            ReadBytes(s, bytes, sizeof(int));
            return BitConverter.ToInt32(bytes, 0);
        }

        public static float ReadFloat(Socket s)
        {
            var bytes = new byte[sizeof(float)];
            ReadBytes(s, bytes, sizeof(float));
            return BitConverter.ToSingle(bytes, 0);
        }

        public static void SendInt(Socket s, int value)
        {
            var bytes = BitConverter.GetBytes(value);
            s.Send(bytes);
        }
    }
}
