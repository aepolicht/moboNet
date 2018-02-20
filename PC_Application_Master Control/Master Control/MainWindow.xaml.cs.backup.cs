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

namespace Master_Control
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        bool ledState = false;
        static SerialPort serialPort1;
        byte[] buffer;
        private System.Windows.Forms.NotifyIcon m_notifyIcon;
        byte groupNum, modCode, redVal, greenVal, blueVal;

        public MainWindow()
        {
            InitializeComponent();

            serialPort1 = new SerialPort();

            serialPort1.PortName = SerialPort.GetPortNames()[0];
            serialPort1.BaudRate = 115200;
            serialPort1.Open();

            buffer = new byte[6];
            buffer[0] = 0xAA;
            
            m_notifyIcon = new System.Windows.Forms.NotifyIcon();
            m_notifyIcon.BalloonTipText = "The Light Command has been minimised. Click the tray icon to show.";
            m_notifyIcon.BalloonTipTitle = "Light Command";
            m_notifyIcon.Text = "Light Command";
            m_notifyIcon.Icon = new System.Drawing.Icon("TheAppIcon.ico");
            m_notifyIcon.Click += new EventHandler(m_notifyIcon_Click);

        }

        private WindowState m_storedWindowState = WindowState.Normal;
        void OnStateChanged(object sender, EventArgs args)
        {
            if (WindowState == WindowState.Minimized)
            {
                Hide();
                if (m_notifyIcon != null)
                    m_notifyIcon.ShowBalloonTip(2000);
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
            write_Array(0,0,0);
            if (serialPort1.IsOpen) serialPort1.Close();
            m_notifyIcon.Dispose();
            m_notifyIcon = null;
        }

        private void Colors_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            switch(Colors.SelectedIndex)
            {
                case 1:
                    write_Array(255,0,0);
                    break;
                case 2:
                    write_Array(0,255,0);
                    break;
                case 3:
                    write_Array(0,0,255);
                    break;
                default:
                    Console.WriteLine("Invalid selection. Please select 1, 2, or 3.");
                    break;
            }            
        }

        public void write_Array(int r, int g, int b)
        {
            int k = 6;
            if (ledState)
                for (int i = 0; i < 24; i++)
                {
                    buffer[k++] = (byte)((r * 255) / 255);
                    buffer[k++] = (byte)((g * 255) / 255);
                    buffer[k++] = (byte)((b * 255) / 255);
                }
            serialPort1.Write(buffer, 0, 81);
        }

        private void ListBox_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {

        }

        private void Black_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Purple_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Teal_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
