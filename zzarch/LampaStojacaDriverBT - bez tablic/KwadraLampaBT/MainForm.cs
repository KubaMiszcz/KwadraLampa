using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using KwadraLampaBT.Properties;
using System.IO.Ports;
using System.Xml;

namespace KwadraLampaBT
{
    public partial class MainForm : Form
    {
        public String DataReceived;

        private int _baudRate=9600;

        public MainForm()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            DataReceived = "";
            serialPort1.BaudRate = 9600;

            List<string> portList = new List<string>(SerialPort.GetPortNames());
            if (portList.Count != 0)
            {
                foreach (var VARIABLE in portList)
                {
                    comboBox1.Items.Add(VARIABLE);
                }
                comboBox1.SelectedIndex = 0;
                serialPort1.PortName = comboBox1.Text; // nazwa portu gdzie jest Arduino podłączone
            }
        }
        private void cbChosenSerial_SelectedIndexChanged(object sender, EventArgs e)
        {
            serialPort1.PortName = comboBox1.Text; // nazwa portu gdzie jest Arduino podłączone
        }


        private void button1_Click(object sender, EventArgs e)
        {
            Button btn=sender as Button;
            if (btn.Tag.ToString() == "off")
            {
                btn.BackgroundImage = Properties.Resources.tglON;
                btn.Tag = "on";
                SerialSend("::L1:onxxxx");

            }
        else
            {
                btn.BackgroundImage = Properties.Resources.tglOFF;
                btn.Tag = "off";
                SerialSend("::L1:offxxx");
            }

        }


        private void SerialSend(String str)
        {
            try
            {
                serialPort1.Write(str);
            }
            catch (Exception ex)
            {
                Report(ex);
            }
        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            //TrackBar tb=sender as TrackBar;
            //label1.Text = tb.Value.ToString();
        }

        private void knobControl1_ValueChanged(object Sender)
        {
            if (knobControl1.Value==256)
            {
                knobControl1.Value = 255;
            }
            String str = knobControl1.Value.ToString();
            label1.Text = str;
            if (str.Length==1)
            {
                str = "xx" + str;
            }
            else if(str.Length==2)
            {
                str = "x" + str;
            }
            SerialSend("::L1:PWM"+str);
        }


        void Report(Exception ex)
        {
            MessageBox.Show(ex.Message + "\n\n" + ex.GetType() + "\n\n" + ex.StackTrace);
        }
        private void Form1_FormClosed(object sender, FormClosedEventArgs e) // instrukcje jakie mają się wykonac w przypadku wyłączenia okna programu
        {
            serialPort1.Close();
        }
        private void ClosePort(object sender, EventArgs e)
        {
            serialPort1.Close();
            UpdatePortStatus();
        }

        void UpdatePortStatus()
        {
            if (serialPort1.IsOpen)
            {
                lbStatusPortu.Text = "otwarty";
                btnOpenClosePort.Text = "zamknij";
            }
            else
            {
                lbStatusPortu.Text = "zam,kniety";
                btnOpenClosePort.Text = "otworz";
            }
        }


        private void button2_Click(object sender, EventArgs e)
        {
            try
            {

                if (!serialPort1.IsOpen) // Jeżeli port nie jest otwarty, otwieramy go
                {
                    serialPort1.PortName = comboBox1.Text;
                    serialPort1.Open(); // otwarcie portu
                }
                else // Jeżeli port  jest otwarty, zamykamy go
                {
                    this.BeginInvoke(new EventHandler(ClosePort)); // wywołanie metody
                }
                UpdatePortStatus();

            }
            catch (UnauthorizedAccessException)
            {
                MessageBox.Show("Wybrany port jest zajety");
            }
            catch (Exception ex)
            {
                Report(ex);
            }
        }

        private void btnReceive_Click(object sender, EventArgs e)
        {
            if (serialPort1.BytesToRead>0)
            {
                String str= serialPort1.ReadLine();
                String str2 = serialPort1.ReadExisting();
                richTextBox1.Text = str;
                richTextBox2.Text = str2 + str;
            }
        }

        private void knobControl1_Load(object sender, EventArgs e)
        {

        }
    }
}
