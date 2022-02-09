using System;
using System.Collections.Generic;
using System.Text;
using FTD2XX_NET;
using System.Threading;
using System.Web.Script.Serialization;

namespace AMIStat
{

    public enum COMM_CHANNEL
    {
        UNKNOWN = 'K',
        COMMAND = 'A',
        EVENT = 'E',
        BIN = 'B',
        DEBUG = 'Z',
        HEARTBEAT = 'H',
        FILE = 'F',
        UPDATE = 'Q',
        UPGRADE = 'P',
        UPLOAD = 'U',
    };
    public class BatteryStatus
    {
        public int SOC;
        public int Voltage;
        public bool Charging;
        public bool wasCharging;
        public double avgPercentage;
        public double avgVoltage;
        public bool avgIsValid;

    }


    public class DeviceInfo
    {
        public string ProductNumber;
        public string SerialNumber;
        public string FWVersion;
        public int ProductType;
        public bool ConnectionStatus;
        public bool DeviceWasConnected;

    }
    public static class AMIJsonCommands
    {
        public const string GetBatteryStatusID = "GetBatteryStatus";
        public const string GetDeviceInfoID = "GetDeviceInfo";
        

    }

    public delegate void evtOnReceived(COMM_CHANNEL chn,string EvtStatus, byte[] b, string s,string ID, Dictionary<string, object> jSon);
    public delegate void evtNewDeviceConnected(DeviceInfo di);
    public delegate void evtChargingValueIsValid();
    class TTLAMICom
    {
        const int BatteryPrecentageAverageWindowSize = 10;
        public int[] BatteryPercentageArray = new int[BatteryPrecentageAverageWindowSize];
        public int[] BatteryVoltgeArray = new int[BatteryPrecentageAverageWindowSize];
        public int AvgCounter => avgIsValid ? BatteryPrecentageAverageWindowSize : bpa;
        int bpa = 0;
        bool _avgIsValid;
        public bool avgIsValid
        {
            get => _avgIsValid;
            set
            {
                _avgIsValid = value;
                 bpa = -1;
               // for (int i = 0; i < BatteryVoltgeArray.Length; i++)
                  //  BatteryVoltgeArray[i] = BatteryPercentageArray[i] = 0;
            }
        }

        public event evtOnReceived OnReceived;
        public event evtNewDeviceConnected OnNewDeviceConnected;
        public event evtChargingValueIsValid OnChargingValueIsValid;
        
        public delegate void evtLog(string log);
        public event evtLog OnLog;
        FTDI ftd = new FTDI();
        public bool EnableDebugLog = false;
        void Log(string s)
        {
            if(!EnableDebugLog)return;
            if (OnLog != null && !closing)
                OnLog(s);
        }


        EventWaitHandle waitHandle=null;

        private bool _done;

        private void PortListener(EventWaitHandle waitHandle)
        {
            byte[] bb = new byte[1];
            List<byte> b = new List<byte>();
            while (true)
            {
                waitHandle.WaitOne();
                if (_done)
                {
                    break;
                }
                ftd.SetTimeouts(1, 10);
                b.Clear();
                uint br = 0;
                do
                {
                    ftd.Read(bb, 1, ref br);
                    if (br > 0)
                        b.Add(bb[0]);
                } while (br != 0);
                string s = Encoding.ASCII.GetString(b.ToArray(), 0, b.Count);
                Log("Received : " + s);
                Pars(b.ToArray());
                
            }
        }

        public void StartListening()
        {
            _done = false;
            new Thread(() => PortListener(waitHandle)).Start();
        }

        public void StopListening()
        {
            try
            {
                if (waitHandle == null) return;
                _done = true;
                waitHandle.Set();
            }
            catch
            {

            }
        }
        public FTDI.FT_DEVICE_INFO_NODE[] DeviceList
        {
            get
            {
                uint dc = 0;
                FTDI.FT_STATUS r;
                ftd.GetNumberOfDevices(ref dc);
                FTDI.FT_DEVICE_INFO_NODE[] dl = new FTDI.FT_DEVICE_INFO_NODE[dc];
                ftd.GetDeviceList(dl);
                return dl;
            }
        }
        public string[] DeviceNames => Array.ConvertAll(DeviceList, x => x.SerialNumber);

        public TTLAMICom()
        {
            
        }
     

        const byte END = 0xC0; // indicates end of packet
        const byte ESC = 0xDB; // indicates byte stuffing
        const byte ESC_END = 0xDC;  // ESC ESbEND means END data byte
        const byte ESC_ESC = 0xDD; // ESC ESbESC means ESC data byte


        public void cmdGetBatteryStatus()
        {
            send(COMM_CHANNEL.COMMAND, "{\n\"ID\": \"GetBatteryStatus\",\n\"Content\":{}\n}");

        }
        public void cmdSwitchToServicePort()
        {
            send(COMM_CHANNEL.COMMAND, "{\n\"ID\": \"SwitchToServicePort\",\n\"Content\":{}\n}");
        }
        public void cmdGetDeviceInfo()
        {
            send(COMM_CHANNEL.COMMAND, "{\n\"ID\": \"GetDeviceInfo\",\n\"Content\":{}\n}");
        }
        public void cmdSetRelay(bool On)
        {
            send(COMM_CHANNEL.COMMAND, "{\n\"ID\": \"SetVariable\",\n\"Content\":{\"Name\": \"AC_POWER_EN\", \"Value\": " + (On?"1":"0") +"}\n}");
        }

     
        void send(COMM_CHANNEL chn, string s)
        {
            int retCode = 0;
            // send start of frame
            byte[] buf = Encoding.ASCII.GetBytes(s);
            List<byte> r = new List<byte>();
            r.Add(END);
            r.Add((byte)chn);
            foreach (byte b in buf)
            {
                switch (b)
                {
                    case ESC:
                        r.Add(ESC);
                        r.Add(ESC_ESC);
                        break;
                    case END:
                        r.Add(ESC);
                        r.Add(ESC_END);
                        break;
                    default:
                        r.Add(b);
                        break;
                }
            }
            r.Add(END);
            uint wc = 0;
            ftd.Write(r.ToArray(),  r.Count,ref wc);
            string log=Encoding.ASCII.GetString(r.ToArray());
            Log("Send:"+log);
        }

        List<byte> recBytes = new List<byte>();


        enum SlipStates {  WaitForStart,GetData, CheckEsc ,GetChannel};
        SlipStates State = SlipStates.WaitForStart;
        COMM_CHANNEL r_chn;
        int waitToStableBatteryInfoCounter = 0;

        public BatteryStatus BatteryStatus = new BatteryStatus();
        public DeviceInfo DeviceInfo = new DeviceInfo();
        JavaScriptSerializer json_serializer = new JavaScriptSerializer();
        private void Tick(byte b)
        {
            switch (State)
            {
                case SlipStates.WaitForStart:     // start of frame and end of frame
                    if (b == END)
                    {
                        recBytes.Clear();
                        State = SlipStates.GetChannel;
                    }
                    break;

                case SlipStates.GetChannel:     // escape character
                    r_chn = (COMM_CHANNEL)b;
                    State = SlipStates.GetData;
                    break;

                case SlipStates.GetData:     // escape character
                    if (b == ESC)
                    {
                        State = SlipStates.CheckEsc;
                    }
                    else
                    if (b == END)
                    {
                        string sj = Encoding.UTF8.GetString(recBytes.ToArray());
                        Dictionary<string, object> jobj = null;
                        string ID = "";
                        string evt = "";
                        sj = sj.Replace("\0", "");
                        if (r_chn == COMM_CHANNEL.EVENT)
                        {
                            if (sj.Contains("ErrorStatusChange"))
                            {
                                cmdSwitchToServicePort();
                                cmdGetDeviceInfo();
                            }
                        }
                        if (r_chn == COMM_CHANNEL.HEARTBEAT)
                        {
                            cmdGetBatteryStatus();
                        }
                        else
                        {
                            try
                            {
                                jobj = (Dictionary<string, object>)json_serializer.DeserializeObject(sj);
                                if (jobj.ContainsKey("ID"))
                                {
                                    ID = (string)jobj["ID"];
                                    Dictionary<string, object> content = null;
                                    if (jobj.ContainsKey("Content"))
                                        content = (Dictionary<string, object>)jobj["Content"];
                                    switch (ID)
                                    {
                                        case AMIJsonCommands.GetBatteryStatusID:
                                            waitToStableBatteryInfoCounter++;
                                            bool wasCharging = BatteryStatus.Charging;
                                            bool charging = (bool)content["Charging"];
                                            if (waitToStableBatteryInfoCounter > 2)
                                            {
                                                if (!wasCharging && charging)
                                                    evt = "ChargerConnected";
                                                if (wasCharging && !charging)
                                                    evt = "ChargerDisconnected";
                                            }
                                            if(bpa<0)
                                            {
                                                bpa = 0;
                                                break;
                                            }
                                            BatteryStatus = new BatteryStatus { wasCharging= wasCharging,SOC = (int)content["SOC"], Voltage = (int)content["Voltage"], Charging = charging };
                                            BatteryPercentageArray[bpa] = BatteryStatus.SOC;
                                            BatteryVoltgeArray[bpa] = BatteryStatus.Voltage;
                                            
                                            bpa++;
                                            if (bpa >= BatteryPercentageArray.Length)
                                            {
                                                if(!avgIsValid)
                                                {
                                                    avgIsValid = true;
                                                    if (OnChargingValueIsValid != null && !closing)
                                                        OnChargingValueIsValid();
                                                }
                                            }
                                            bpa %= BatteryPercentageArray.Length;
                                            int sum = 0;
                                            for (int i = 0; i < BatteryPercentageArray.Length; i++)
                                                sum += BatteryPercentageArray[i];
                                            BatteryStatus.avgPercentage = ((double)sum) / BatteryPercentageArray.Length;
                                            sum = 0;
                                            for (int i = 0; i < BatteryPercentageArray.Length; i++)
                                                sum += BatteryVoltgeArray[i];
                                            BatteryStatus.avgVoltage = ((double)sum) / BatteryPercentageArray.Length;
                                            BatteryStatus.avgIsValid = avgIsValid;
                                            if(!DeviceInfo.ConnectionStatus)
                                            {
                                                evt = "DeviceConnected";
                                                waitToStableBatteryInfoCounter = 0;
                                                avgIsValid = false;
                                            }
                                            DeviceInfo.ConnectionStatus = true;
                                            break;
                                        case AMIJsonCommands.GetDeviceInfoID:
                                            bool devicewasconnected = DeviceInfo.ConnectionStatus;


                                            if (!devicewasconnected)
                                            {
                                                evt = "DeviceConnected";
                                                waitToStableBatteryInfoCounter = 0;
                                                avgIsValid = false;
                                            }

                                            DeviceInfo = new DeviceInfo { DeviceWasConnected=devicewasconnected, ConnectionStatus = true, ProductNumber = (string)content["ProductNumber"], FWVersion = (string)content["FwMainVersion"], SerialNumber = (string)content["SerialNumber"], ProductType = (int)content["ProductType"] };


                                            if (OnNewDeviceConnected != null && !devicewasconnected && !closing)
                                                OnNewDeviceConnected(DeviceInfo);
                                            break;
                                    }
                                }
                            }
                            catch
                            {
                                recBytes.Clear();
                                State = SlipStates.WaitForStart;
                                break;
                            }



                        }


                        if (OnReceived != null && !closing)
                        {
                            OnReceived(r_chn,evt, recBytes.ToArray(), sj,ID,jobj);
                        }
                        recBytes.Clear();
                        State = SlipStates.WaitForStart;
                    }
                    else recBytes.Add(b);
                    break;
                case SlipStates.CheckEsc:     // escape character
                    if (b == ESC_END)
                    {
                        recBytes.Add(END);
                    }
                    else if (b == ESC_ESC)
                    {
                        recBytes.Add(ESC);
                    }
                    State = SlipStates.GetData;
                    break;

            }


        }
        private void Pars(byte[] RecBytes)
        {
            for (int i = 0; i < RecBytes.Length && !closing; i++)
            {

                Tick(RecBytes[i]);
            }
        }
 


        public bool isOpen => ftd.IsOpen;
        bool closing = false;
        public bool Open(string serial)
        {

            if (ftd.IsOpen) return false;


            FTDI.FT_STATUS r;

            r = ftd.OpenBySerialNumber(serial);
            r = ftd.ResetDevice();

            r = ftd.SetFlowControl(FTDI.FT_FLOW_CONTROL.FT_FLOW_RTS_CTS, 0, 0);
            r = ftd.SetBaudRate(921600);
            r = ftd.SetBitMode(0, 0);
            r = ftd.SetDataCharacteristics(FTDI.FT_DATA_BITS.FT_BITS_8, FTDI.FT_STOP_BITS.FT_STOP_BITS_1, FTDI.FT_PARITY.FT_PARITY_NONE);
            r = ftd.SetRTS(true);
            waitHandle = new EventWaitHandle(false, EventResetMode.AutoReset, "");
            r = ftd.SetEventNotification(FTDI.FT_EVENTS.FT_EVENT_RXCHAR, waitHandle);
            StartListening();
            return true;
        }
        public void Close()
        {
            try
            {
                if (ftd.IsOpen)
                {
                    closing = true;
                    StopListening();
                    ftd.Close();
                    closing = false;
                }
            }
            catch
            {
            }
        }
    }
}
