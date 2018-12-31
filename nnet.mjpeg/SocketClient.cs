using System;
using System.Drawing;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace nNet.mjpeg
{
    public delegate void OnNewFrame(Bitmap b, Region[] regions);
    public delegate void OnStateChanged(string message);

    public class SocketClient
    {
        private readonly string _endPoint;

        private Thread _thread;
        private bool _stop;

        private Bitmap _currentImage;
        private Bitmap _nextImage = null;
        private object _nextImageLock = new object();

        public event OnNewFrame OnNewFrame;
        public event OnStateChanged OnStateChanged;

        public SocketClient(string endPoint)
        {
            if (string.IsNullOrEmpty(endPoint)) throw new ArgumentException(nameof(endPoint));
            _endPoint = endPoint;
        }

        private IPEndPoint ParseEndpoint(string endPoint)
        {
            var splited = endPoint.Split(':');
            var host = Dns.GetHostEntry(splited[0]);
            return new IPEndPoint(host.AddressList[0], int.Parse(splited[1]));
        }

        public void Start()
        {
            _stop = false;
            _thread = new Thread(Worker);
            _thread.Start();
        }

        public void Stop()
        {
            _stop = true;
        }

        public void PutImage(Bitmap b)
        {
            lock (_nextImageLock)
            {
                if (_nextImage == null || _nextImage.Width != b.Width || _nextImage.Height != b.Height)
                    _nextImage = new Bitmap(b.Width, b.Height);
                BitmapUtils.ResizeToBitmap(b, _nextImage);
            }
        }

        private void Worker()
        {
            var endPoint = ParseEndpoint(_endPoint);
            var socket = new Socket(endPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            try
            {
                socket.Connect(endPoint);
            }
            catch (Exception e)
            {
                OnStateChanged?.Invoke(e.Message);
                return;
            }

            var inputWidth = SocketUtils.ReadInt(socket);
            var inputHeight = SocketUtils.ReadInt(socket);
            var inputChannels = SocketUtils.ReadInt(socket);
            _currentImage = new Bitmap(inputWidth, inputHeight);

            OnStateChanged?.Invoke(string.Format("Connected. Input: {0}x{1}x{2}", inputWidth, inputHeight, inputChannels));

            var input = new byte[inputWidth * inputHeight * inputChannels * sizeof(float)];

            try
            {
                while (!_stop)
                    Process(socket, input, inputWidth, inputHeight);

                OnStateChanged?.Invoke("Disconnected.");
            }
            catch (Exception e)
            {
                OnStateChanged?.Invoke(e.Message);
            }
            finally
            {
                socket.Close();
            }
        }

        private void Process(Socket socket, byte[] input, int inputWidth, int inputHeight)
        {
            int regionCount, r;
            Region[] regions;

            lock (_nextImageLock)
            {
                if (_nextImage == null)
                    _nextImage = new Bitmap(inputWidth, inputHeight);
                BitmapUtils.ResizeToBitmap(_nextImage, _currentImage);
            }

            SocketUtils.SendInt(socket, _currentImage.Width);
            SocketUtils.SendInt(socket, _currentImage.Height);

            BitmapUtils.BitmapToNormalizedFloats(input, _currentImage);
            socket.Send(input);

            regionCount = SocketUtils.ReadInt(socket);
            regions = null;
            if (regionCount > 0)
            {
                regions = new Region[regionCount];
                for (r = 0; r < regionCount; r++)
                {
                    regions[r] = new Region()
                    {
                        ClassIndex = SocketUtils.ReadInt(socket),
                        Probability = SocketUtils.ReadFloat(socket),
                        X = SocketUtils.ReadFloat(socket),
                        Y = SocketUtils.ReadFloat(socket),
                        W = SocketUtils.ReadFloat(socket),
                        H = SocketUtils.ReadFloat(socket)
                    };
                }
            }

            OnStateChanged?.Invoke(string.Format("Forwarded, regions: {0}", regionCount));

            if (!_stop && OnNewFrame != null)
                OnNewFrame(_currentImage, regions);
        }
    }
}
