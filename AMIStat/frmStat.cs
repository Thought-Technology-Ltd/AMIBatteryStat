using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Diagnostics;
using System.Threading;
using System.IO;

namespace AMIStat
{
    public partial class frmMonitor : Form
    {
        TTLAMICom amicom = new TTLAMICom();
        public frmMonitor()
        {
            InitializeComponent();
            amicom.OnLog += onlog;
            amicom.OnReceived += Amicom_OnReceived;
            amicom.OnChargingValueIsValid += Amicom_OnChargingValueIsValid;
            ico = this.Icon;
        }

        private void Amicom_OnChargingValueIsValid()
        {
          

        }

        Icon ico;
        private void Amicom_OnReceived(COMM_CHANNEL chn,string evt, byte[] b, string s, string ID, Dictionary<string, object> jSon)
        {
            try
            {
                this.Invoke((MethodInvoker)delegate
                {
                    if (ID == AMIJsonCommands.GetBatteryStatusID)
                    {
                        RefreshValues();
                        lblStatus.Text = "Service port: Connected";
                        lblStatus.ForeColor = Color.Green;
                        timer2.Enabled = true;
                        timer1.Enabled = false;
                        timer1.Enabled = true;
                        if(LogRequested)
                        {
                            AMIBatteryLog bt = new AMIBatteryLog("UserRequest", amicom.DeviceInfo, amicom.BatteryStatus);
                            AddLogRecord(bt);
                            btnLog.Enabled = true;
                            LogRequested = false;
                        }


                    }
                    if (ID == AMIJsonCommands.GetDeviceInfoID)
                    {
                        lblSerial.Text = "Serial: " + amicom.DeviceInfo.SerialNumber;
                    }
                   
                    if (!Connected)
                    {
                        LogRequested = false;
                        btnLog.Enabled = true;
                        amicom.cmdGetDeviceInfo();
                    }
                    else if (evt!="")
                    {
                        AMIBatteryLog bt = new AMIBatteryLog(evt, amicom.DeviceInfo, amicom.BatteryStatus);
                        try
                        {
                            AddLogRecord(bt);
                        }
                        catch
                        {
                            CriticalFileError();
                        }
                    }
                });
            }
            catch
            {

            }
        }
        
        void RefreshValues()
        {
            if (amicom.avgIsValid && ShowWarning)
            {
                lblVoltage.Text = lblLevel.Text = "Waiting ... ";
                return;
            }
            lblVoltage.Text = "Voltage: " + (amicom.avgIsValid ? +
                (chkAvg.Checked ? (int)Math.Round(amicom.BatteryStatus.avgVoltage ): amicom.BatteryStatus.Voltage) + " mV" 
                : "Waiting ... "+amicom.AvgCounter) ;
            lblLevel.Text = "Level: " + amicom.BatteryStatus.SOC +" %";
                /*(amicom.avgIsValid ?
                (chkAvg.Checked? amicom.BatteryStatus.avgPercentage : amicom.BatteryStatus.SOC) + " %"
                :"Waiting ... "+amicom.AvgCounter);*/
            lblPower.Text = "Power: " + (amicom.BatteryStatus.Charging? "Charging":"Disconnected");
            progLevel.Value = amicom.BatteryStatus.SOC;
        }
        void onlog(string s)
        {
            try
            {
                this.Invoke((MethodInvoker)delegate
                {
                    txtLog.Text = s + "\r\n" + txtLog.Text;
                });
            }
            catch
            {

            }
        }

        private void btnExit_Click(object sender, EventArgs e)
        {
            amicom.Close();
            this.Enabled = false;
            while (amicom.isOpen)
                Application.DoEvents();
            Application.Exit();
        }

        bool formLoaded = false;
        const int waveVOssfets = 300;
        string LogDir => Application.StartupPath + "\\Log\\";
        string LogFile => LogDir + "log.csv";
        
        private void frmMonitor_Load(object sender, EventArgs e)
        {

            comboBox1.DropDown += ComboBox1_DropDown;
            chkRestrictedevent.Checked=Settings1.Default.RestrictedEventEnable ;

            amicom.EnableDebugLog = chkUartLogEnable.Checked = Settings1.Default.EnableDebugLog;
            chkAvg.Checked = Settings1.Default.AverageEnable;
            btnLog.Enabled = !chkUartLogEnable.Checked;
            txtLog.Font = new Font(FontFamily.GenericMonospace, txtLog.Font.Size);
            FillPorts();
            string pn = Settings1.Default.PortName;
            if (pn != "" && comboBox1.Items.Contains(pn))
                comboBox1.SelectedItem = pn;



                formLoaded = true;
            if (!Directory.Exists(LogDir))
                Directory.CreateDirectory(LogDir);
            if (!File.Exists(LogFile))
            {
                try
                {
                    File.WriteAllText(LogFile, AMIBatteryLog.LogHeader);
                    
                }
                catch
                {
                    CriticalFileError();
                }
            }
            else
            {
                File.Copy(LogFile,LogDir+ "Log_Backup_" + DateTime.Now.Ticks + ".csv");
            }

            if (File.Exists("config.txt"))
            {
                string s = File.ReadAllText("config.txt");
                s = s.ToLower().Replace(" ", "");
                /*
                 * mode=operatordefault
                 * mode=operatoroverride
                 * mode=admin
                 */
                if (s.Contains("mode=operatordefault") && !s.Contains("//mode=operatordefault"))
                {
                    chkUartLogEnable.Checked = false;
                    chkAvg.Checked = true;
                    chkRestrictedevent.Checked = true;
                }
                else if (s.Contains("mode=operatoroverride") && !s.Contains("//mode=operatoroverride"))
                {

                }
                else if (s.Contains("mode=admin") && !s.Contains("//mode=admin"))
                {
                    chkUartLogEnable.Enabled = chkAvg.Enabled = chkRestrictedevent.Enabled = true;
                }
            }
        }
        bool errorIsIssued = false;
        void CriticalFileError()
        {
            if (errorIsIssued || !btnClose.Enabled) return;
            errorIsIssued = true;
            MessageBox.Show("The log file is in use.\nplease close the log file and start application again.", "Critical error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            ClosePort();
            /*for (int i = 0; i < 100;i++)
            {
                Thread.Sleep(1);
                Application.DoEvents();
            }*/
            errorIsIssued = false;
        }
        List<AMIBatteryLog> Logs = new List<AMIBatteryLog>();
        void LoadLogs()
        {
            string[] ss = File.ReadAllLines(LogFile);
            for (int i = 1; i < ss.Length; i++)
                Logs.Add(new AMIBatteryLog(ss[i]));
        }
        
        private void ComboBox1_DropDown(object sender, EventArgs e)
        {
            FillPorts();
        }

        void FillPorts()
        {
            var ss = amicom.DeviceNames;
            comboBox1.Items.Clear();
            comboBox1.Items.AddRange(ss);
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            if (amicom.Open(comboBox1.Text))
            {

                comboBox1.Enabled = btnOpen.Enabled = false;
                btnClose.Enabled = true;
              
                Settings1.Default.PortName = comboBox1.Text;
                Settings1.Default.Save();

            }
        }
void ClosePort()
        {
            this.Enabled = false;
            amicom.Close();
            while (amicom.isOpen)
                Application.DoEvents();
            comboBox1.Enabled = btnOpen.Enabled = true;
            btnClose.Enabled = false;
            this.Enabled = true;
        }
        private void btnClose_Click(object sender, EventArgs e)
        {

            ClosePort();

        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (amicom.EnableDebugLog) return;
            LogRequested = true;
            btnLog.Enabled = false;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            amicom.cmdSwitchToServicePort();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            amicom.cmdGetBatteryStatus();
            //Text = amicom.port.RtsEnable ? "On" : "Off";
        }

        private void frmMonitor_FormClosed(object sender, FormClosedEventArgs e)
        {
//            amicom.StopListening();
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void button4_Click(object sender, EventArgs e)
        {
            amicom.cmdSetRelay(true);

        }

        private void button5_Click(object sender, EventArgs e)
        {
            amicom.cmdSetRelay(false);
        }

        private void tableLayoutPanel2_Paint(object sender, PaintEventArgs e)
        {

        }
        private void timer2_Tick(object sender, EventArgs e)
        {
            lblStatus.ForeColor = Color.Black;
            timer2.Enabled = false;
        }
        int logCountDuringConnection = 0;
        bool PendingLog = false;
        void SaveLog(AMIBatteryLog log)
        {
            if (chkRestrictedevent.Checked) log.Event = "DeviceConnected";
            logCountDuringConnection++;
            File.AppendAllText(LogFile, log.Log);
            if (!amicom.EnableDebugLog)
            {
                txtLog.Text = log.LogText + txtLog.Text;
              /*  for (int i = 0; i < amicom.BatteryVoltgeArray.Length; i++)
                    txtLog.Text = amicom.BatteryVoltgeArray[i] + " " + txtLog.Text;
                for (int i = 0; i < amicom.BatteryVoltgeArray.Length; i++)
                    txtLog.Text = amicom.BatteryPercentageArray[i] + " " + txtLog.Text;*/

            }
        }
        void AddLogRecord(AMIBatteryLog log)
        {
            if (PendingLog) return;
            if(log.Event == "DeviceConnected")
            {
                logCountDuringConnection = 0;
            }
         
            if (log.Event == "UserRequest" || !chkRestrictedevent.Checked  || 
                ((log.Event == "ChargerDisconnected" ||log.Event == "DeviceConnected") && !log.isCharging && logCountDuringConnection < 1))
            {
                if (amicom.avgIsValid)
                {
                    SaveLog(log);
                }
                else
                    PendingLog = true;
            }
        }
        bool Connected
        {
            get => amicom.DeviceInfo.ConnectionStatus;
            set => amicom.DeviceInfo.ConnectionStatus = value;
        }
        bool LogRequested = false;
        private void timer1_Tick(object sender, EventArgs e)
        {
            lblStatus.ForeColor = Color.Red;
            lblStatus.Text = "Service port: Disconnected";
            amicom.avgIsValid = false;

            if (Connected)//Disconnection event
            {
                LogRequested = false;
                btnLog.Enabled = true;

                AMIBatteryLog b = new AMIBatteryLog("DeviceDisconnected", amicom.DeviceInfo, amicom.BatteryStatus);
                try
                {
                    AddLogRecord(b);
                }
                catch
                {
                    CriticalFileError();
                }
            }
            Connected = false;

        }

        private void frmMonitor_FormClosing(object sender, FormClosingEventArgs e)
        {
            ClosePort();
        }

        private void chkUartLogEnable_CheckedChanged(object sender, EventArgs e)
        {
            if (formLoaded)
            {
                amicom.EnableDebugLog = Settings1.Default.EnableDebugLog = chkUartLogEnable.Checked;
                btnLog.Enabled = !chkUartLogEnable.Checked;
                Settings1.Default.Save();
            }
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            txtLog.Text = "";
        }

        private void chkAvg_CheckedChanged(object sender, EventArgs e)
        {
            if (formLoaded)
            {
                Settings1.Default.AverageEnable = chkAvg.Checked;
                Settings1.Default.Save();
            }
        }

        private void chkRestrictedevent_CheckedChanged(object sender, EventArgs e)
        {
            if (formLoaded)
            {
                Settings1.Default.RestrictedEventEnable = chkRestrictedevent.Checked;
                Settings1.Default.Save();
                if(!chkRestrictedevent.Checked)
                {
                    lblWarning.Visible = false;
                }
            }
        }
        bool ShowWarning=> (chkRestrictedevent.Checked && Connected && amicom.BatteryStatus.Charging);

        private void timer3_Tick(object sender, EventArgs e)
        {

            if (ShowWarning)
            {
                lblWarning.Visible = !lblWarning.Visible;
            }
            else
                lblWarning.Visible = false;


            if (PendingLog && amicom.avgIsValid)
            {
                AMIBatteryLog bt = new AMIBatteryLog("DeviceConnected", amicom.DeviceInfo, amicom.BatteryStatus);
                PendingLog = false;
                AddLogRecord(bt);
            }


        }
    }
    class AMIBatteryLog
    {
        public string Serial;
        public int Voltage;
        public int Percentage;
        public string ChargerStatus;
        public DateTime DateTime;
        public string Event;
        public string FWVersion;
        public bool isCharging;


        public static string LogHeader => "Event,DateTime,DeviceSerial,Voltage,Percentage,Charger,FWVersion";
        public string Log => "\r\n" + Event + "," + DateTime.ToString() + "," + Serial + "," + Voltage + "," + Percentage + "," + ChargerStatus + "," + FWVersion;
        public string LogText =>string.Format("\r\n{0,-20}{1,-15}{2,-10}{3,-6}{4,-4}{5,-15}{6,-15}", Event ,  DateTime.ToString() , Serial , Voltage , Percentage , ChargerStatus,FWVersion);
        public AMIBatteryLog(string Event,DeviceInfo di, BatteryStatus bs)
        {
            this.Event = Event;
            Serial = di.SerialNumber;
            FWVersion = di.FWVersion;
            Voltage =(int)Math.Round( bs.avgVoltage);
            Percentage = bs.SOC;// (int)Math.Round(bs.avgPercentage);
            ChargerStatus = bs.Charging ? "Charging" : "Discoonected";
            isCharging = bs.Charging;
            DateTime = DateTime.Now;
        }

        public AMIBatteryLog(string logRecord)
        {
            string[] ss = logRecord.Split(new string[] { ","}, StringSplitOptions.RemoveEmptyEntries);
            if (ss.Length != 6) return;
            int i = 0;
            this.Event = ss[i++];
            DateTime = Convert.ToDateTime(ss[i++]);
            Serial = ss[i++];
            Voltage = Convert.ToInt32(ss[i++]);
            Percentage= Convert.ToInt32(ss[i++]);
            ChargerStatus = ss[i++];
        }
    }

    }