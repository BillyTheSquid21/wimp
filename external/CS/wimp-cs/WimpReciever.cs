using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using static WimpCS.WimpCore;

namespace WimpCS
{
    public class WimpReciever
    {
        private String m_RecFromName = "127.0.0.1";
        private String m_RecFromDomain = "127.0.0.1";
        private Int32 m_RecFromPort = 8000;
        private bool m_IsRunning = false;
        public WimpReciever(String recFromName, String recFromDomain, Int32 recFromPort)
        {
            m_RecFromName = recFromName;
            m_RecFromDomain = recFromDomain;
            m_RecFromPort = recFromPort;
        }

        public void Start(String processName, String processDomain, Int32 processPort)
        {
            if (this.m_IsRunning)
            {
                return;
            }

            WimpRecieverResult res = (WimpRecieverResult)wimp_start_local_server_reciever_thread(processName, processDomain, processPort, m_RecFromName, m_RecFromDomain, m_RecFromPort);
            if (res != WimpRecieverResult.SUCCESS)
            {
                return;
            }

            m_IsRunning = true;
        }

        public String Name()
        {
            return this.m_RecFromName;
        }

        public String Domain()
        {
            return this.m_RecFromDomain;
        }

        public Int32 Port()
        {
            return this.m_RecFromPort;
        }
    }
}
