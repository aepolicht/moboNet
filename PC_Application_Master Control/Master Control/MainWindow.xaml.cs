using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO.Ports;
using System.Threading;
using Microsoft.DirectX;
using SlimDX;
using SlimDX.XInput;
using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing;
using System.Net.Sockets;
using System.Net;
using System.Text.RegularExpressions;

namespace Master_Control
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        UdpClient udpClient;
        byte[] buffer;
        private System.Windows.Forms.NotifyIcon m_notifyIcon;
        byte redVal, greenVal, blueVal, flash, modCode;
        int intensity=0;

        string ServerIPAddress = "";
        //TcpClient tcpClient = new TcpClient();
        int ServerListeningPort = 8080;

        public MainWindow()
        {
            InitializeComponent();
            
            buffer = new byte[8];
            buffer[0] = (byte) 0xAA;

            m_notifyIcon = new System.Windows.Forms.NotifyIcon();
            m_notifyIcon.BalloonTipText = "The Light Command has been minimised. Click the tray icon to show.";
            m_notifyIcon.BalloonTipTitle = "Light Command";
            m_notifyIcon.Text = "Light Command";
            m_notifyIcon.Icon = new System.Drawing.Icon("TheAppIcon.ico");
            m_notifyIcon.Click += new EventHandler(m_notifyIcon_Click);

            ServerIPAddress = getServerIP();
            if(ServerIPAddress != String.Empty)
            {
                SB_Left.Content = "Connected.";
            }
            else
            {
                SB_Left.Content = "No Server Found.";
            }
        }

        private WindowState m_storedWindowState = WindowState.Normal;
        void OnStateChanged(object sender, EventArgs args)
        {
            if (WindowState == WindowState.Minimized)
            {
                Hide();
                //if (m_notifyIcon != null)
                    //m_notifyIcon.ShowBalloonTip(2000);
            }
            else
                m_storedWindowState = WindowState;
        }

        void OnIsVisibleChanged(object sender, DependencyPropertyChangedEventArgs args)
        {
            CheckTrayIcon();
        }

        void m_notifyIcon_Click(object sender, EventArgs e)
        {
            Show();
            WindowState = m_storedWindowState;
        }
        void CheckTrayIcon()
        {
            ShowTrayIcon(!IsVisible);
        }

        void ShowTrayIcon(bool show)
        {
            if (m_notifyIcon != null)
                m_notifyIcon.Visible = show;
        }

        private void Window_Closing_1(object sender, System.ComponentModel.CancelEventArgs e)
        {
            m_notifyIcon.Dispose();
            m_notifyIcon = null;
        }

        private void Colors_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if(buffer != null)
                switch(Colors.SelectedIndex)
                {
                    case 0:
                        modCode = 0x01;
                        write_Array();
                        break;
                    case 1:
                        modCode = 0x02;
                        write_Array();
                        break;
                    case 2:
                        modCode = 0x03;
                        write_Array();
                        break;
                    case 3:
                        modCode = 0x04;
                        write_Array();
                        break;
                    case 4:
                        modCode = 0x05;
                        write_Array();
                        break;
                    case 5:
                        modCode = 0x06;
                        write_Array();
                        break;
                    default:
                        Console.WriteLine("Invalid selection. Please select 1, 2, or 3.");
                        break;
                }            
        }

        public void write_Array()
        {
            if (buffer != null)
            {
                buffer[0] = 0xA1;
                //buffer[1] = 0x01;
                buffer[1] = modCode;
                buffer[2] = (byte) (redVal);
                buffer[3] = (byte) (greenVal);
                buffer[4] = (byte) (blueVal);
                buffer[5] = (byte) intensity;
                buffer[6] = flash;

                System.Diagnostics.Debug.Write(" Flash: ");
                System.Diagnostics.Debug.WriteLine(flash);
                System.Diagnostics.Debug.Write(" Red: ");
                System.Diagnostics.Debug.WriteLine(redVal);
                System.Diagnostics.Debug.Write(" Green: ");
                System.Diagnostics.Debug.WriteLine(greenVal);
                System.Diagnostics.Debug.Write(" Blue: ");
                System.Diagnostics.Debug.WriteLine(blueVal);
                System.Diagnostics.Debug.WriteLine("~~~~~~~~~~~~~~~~~~~~~~");

                sendDataTCP(buffer);

                /*
                int indata = serialPort1.ReadByte();
                System.Diagnostics.Debug.WriteLine("Data Received:");
                System.Diagnostics.Debug.WriteLine(indata);
                System.Diagnostics.Debug.WriteLine("~~~~~~~~~~~~~~~~~~~~~~");*/

            }
        }
        
        public string getServerIP()
        {
            string ThisIP = LocalIPAddress();
            string LocalPrefix = Regex.Match(ThisIP, @".*\..*\.").ToString();


                for (int i = 0; i < 256; i++)
                {
                    try
                    {
                        var tcpClient = new TcpClient();
                        var result = tcpClient.BeginConnect(LocalPrefix + i.ToString(), ServerListeningPort, null, null);

                        var success = result.AsyncWaitHandle.WaitOne(TimeSpan.FromMilliseconds(10));

                        if (!success)
                        {
                            throw new Exception("Failed to connect.");
                        }

                        tcpClient.ReceiveTimeout = 5;

                        //var tcpClient = new TcpClient(LocalPrefix + i.ToString(), ServerListeningPort);
                        //tcpClient.SendTimeout = 50;
                        //tcpClient.ReceiveBufferSize = 10;

                        byte[] data = System.Text.Encoding.ASCII.GetBytes("Hello?");

                        NetworkStream stream = tcpClient.GetStream();

                        // Send the message to the connected TcpServer. 
                        stream.Write(data, 0, data.Length);

                        // Receive the TcpServer.response. 

                        // Buffer to store the response bytes.
                        data = new Byte[256];

                        // String to store the response ASCII representation.
                        String responseData = String.Empty;

                        // Read the first batch of the TcpServer response bytes.
                        Int32 bytes = stream.Read(data, 0, data.Length);
                        responseData = System.Text.Encoding.ASCII.GetString(data, 0, bytes);

                        if (String.Equals(responseData, "Ack"))
                            return LocalPrefix + i.ToString();

                        // Close everything.
                        stream.Close();
                        tcpClient.Close();
                    }
                    catch (ArgumentNullException e)
                    {
                        //Console.WriteLine("ArgumentNullException: {0}", e);
                    }
                    catch (SocketException e)
                    {
                        //Console.WriteLine("SocketException: {0}", e);
                    }
                    catch (Exception e)
                    {
                        //Console.WriteLine("SocketException: {0}", e);
                    }
                }


            return String.Empty;
        }

        public string LocalIPAddress()
        {
            IPHostEntry host;
            string localIP = "";
            host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (IPAddress ip in host.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    localIP = ip.ToString();
                    break;
                }
            }
            return localIP;
        }

        private static void DataReceivedHandler(
                    object sender,
                    SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;

            //int indata = serialPort1.ReadByte();
            //System.Diagnostics.Debug.WriteLine("Data Received:");
            //System.Diagnostics.Debug.WriteLine(indata);
        }

        private void sendDataTCP(byte[] buffer)
        {
            try
            {
                if (ServerIPAddress == String.Empty)
                {
                    ServerIPAddress = getServerIP();

                    if (ServerIPAddress != String.Empty)
                    {
                        SB_Left.Content = "Connected.";
                    }
                    else
                    {
                        SB_Left.Content = "No Server Found.";
                    }
                }

                TcpClient client = new TcpClient(ServerIPAddress, ServerListeningPort);

                client.ReceiveTimeout = 100;

                NetworkStream stream = client.GetStream();

                // Send the message to the connected TcpServer. 
                stream.Write(buffer, 0, buffer.Length);

                // Receive the TcpServer.response. 

                // Buffer to store the response bytes.
                buffer = new Byte[256];

                // String to store the response ASCII representation.
                String responseData = String.Empty;

                // Read the first batch of the TcpServer response bytes.
                Int32 bytes = stream.Read(buffer, 0, buffer.Length);
                responseData = System.Text.Encoding.ASCII.GetString(buffer, 0, bytes);

                // Close everything.
                stream.Close();
                client.Close();
            }
            catch (ArgumentNullException e)
            {
                //Console.WriteLine("ArgumentNullException: {0}", e);
            }
            catch (SocketException e)
            {
                //Console.WriteLine("SocketException: {0}", e);
                SB_Left.Content = "Connection lost.";
            }
        }

        private void intensitySlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            intensity = (int) (254*intensitySlider.Value / (intensitySlider.Maximum - intensitySlider.Minimum));
            write_Array();
        }

        private void speedSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            flash = (byte)(254 * speedSlider.Value / (speedSlider.Maximum - speedSlider.Minimum));
            write_Array();
        }

        private void redSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            redVal = (byte)(254 * redSlider.Value / (redSlider.Maximum - redSlider.Minimum));
            write_Array();
        }

        private void greenSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            greenVal = (byte)(254 * greenSlider.Value / (greenSlider.Maximum - greenSlider.Minimum));
            write_Array();
        }

        private void blueSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            blueVal = (byte)(254 * blueSlider.Value / (blueSlider.Maximum - blueSlider.Minimum));
            write_Array();
        }

        private void Black_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)0;
            greenVal = (byte)0;
            blueVal = (byte)0;
            write_Array();
        }

        private void Purple_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)150;
            greenVal = (byte)0;
            blueVal = (byte)255;
            write_Array();
        }

        private void Teal_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)0;
            greenVal = (byte)255;
            blueVal = (byte)255;
            write_Array();
        }

        private void A_ON_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = {0xA2, 0x01, 0x01};

            sendDataTCP(tmpbuffer);
        }

        private void A_OFF_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x01, 0x00 };

            sendDataTCP(tmpbuffer);
        }

        private void B_ON_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x02, 0x01 };

            sendDataTCP(tmpbuffer);
        }

        private void B_OFF_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x02, 0x00 };

            sendDataTCP(tmpbuffer);
        }

        private void C_ON_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x03, 0x01 };

            sendDataTCP(tmpbuffer);
        }

        private void C_OFF_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x03, 0x00 };

            sendDataTCP(tmpbuffer);
        }

        private void D_ON_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x04, 0x01 };

            sendDataTCP(tmpbuffer);
        }

        private void D_OFF_Click(object sender, RoutedEventArgs e)
        {
            byte[] tmpbuffer = { 0xA2, 0x04, 0x00 };

            sendDataTCP(tmpbuffer);
        }

        private void White_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte) 255;
            greenVal = (byte) 255;
            blueVal = (byte) 255;
            write_Array();
        }

        private void Red_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)255;
            greenVal = (byte)0;
            blueVal = (byte)0;
            write_Array();
        }

        private void Orange_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)195;
            greenVal = (byte)90;
            blueVal = (byte)0;
            write_Array();
        }

        private void Yellow_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)240;
            greenVal = (byte)255;
            blueVal = (byte)0;
            write_Array();
        }

        private void Green_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)0;
            greenVal = (byte)255;
            blueVal = (byte)0;
            write_Array();
        }

        private void Blue_Click(object sender, RoutedEventArgs e)
        {
            redVal = (byte)0;
            greenVal = (byte)0;
            blueVal = (byte)255;
            write_Array();
        }

        private void ListBoxItem_Selected(object sender, RoutedEventArgs e)
        {

        }

    }
}
