using MjpegProcessor;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace nNet.mjpeg
{
    public partial class MainForm : Form
    {
        private readonly Pen _pen = new Pen(Color.Red, 3);
        private readonly SolidBrush _bursh = new SolidBrush(Color.Red);
        private readonly Font _font = new Font("Arial", 12);

        private MjpegDecoder _mjpeg;
        private SocketClient _client;

        public MainForm()
        {
            InitializeComponent();

            nnetHostTextBox.Text = "127.0.0.1:1000";
            //mjpegTextBox.Text = "http://194.68.122.244:84/mjpg/video.mjpg?COUNTER";
            //mjpegTextBox.Text = "http://95.161.181.201:80/mjpg/video.mjpg?COUNTER";
            mjpegTextBox.Text = "http://50.73.9.194:80/mjpg/video.mjpg";
        }

        private void startButton_Click(object sender, EventArgs e)
        {
            startButton.Enabled = false;
            stopButton.Enabled = true;

            _mjpeg = new MjpegDecoder();
            _mjpeg.FrameReady += mjpeg_FrameReady;
            _mjpeg.ParseStream(new Uri(mjpegTextBox.Text));

            _client = new SocketClient(nnetHostTextBox.Text);
            _client.OnNewFrame += new OnNewFrame(client_OnNewFrame);
            _client.OnStateChanged += new OnStateChanged(client_OnStateChanged);
            _client.Start();
        }

        private void stopButton_Click(object sender, EventArgs e)
        {
            startButton.Enabled = true;
            stopButton.Enabled = false;

            // Clear picture box.
            pictureBox.Image = null;
            pictureBox.InitialImage = null;

            _mjpeg.StopStream();
            _client.Stop();
        }

        private void mjpeg_FrameReady(object sender, FrameReadyEventArgs e)
        {
            _client.PutImage(e.Bitmap);
        }

        private void client_OnNewFrame(Bitmap b, Region[] regions)
        {
            var frame = (Bitmap)b.Clone();
            var w = b.Width;
            var h = b.Height;

            if (regions != null)
            {
                using (var g = Graphics.FromImage(frame))
                {
                    foreach (var region in regions)
                    {
                        var rw = region.W * w;
                        var rh = region.H * h;
                        var x = (region.X * w) - (rw / 2);
                        var y = (region.Y * h) - (rh / 2);
                        g.DrawRectangle(_pen, x, y, rw, rh);
                        g.DrawString(region.ClassIndex.ToString(),
                            _font, _bursh, new PointF(x, y));
                    }
                }
            }
            
            pictureBox.Image = frame;
        }

        private void client_OnStateChanged(string message)
        {
            statusStripLabel.Text = message;
        }
    }
}
