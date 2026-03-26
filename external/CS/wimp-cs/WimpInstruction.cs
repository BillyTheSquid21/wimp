using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

using static WimpCS.WimpCore;

namespace WimpCS
{
    public enum WimpInstructionsCore : UInt64
    {
        EXIT = 1540385UL,
        LOG = 161821UL,
        PING = 1446181UL,
        HANDSHAKE_STATUS = 27140846854UL
    }

    public class WimpInstructionNode
    {
        public struct WimpInstrMeta
        {
            public WimpInstrMeta(_WimpInstrMeta meta)
            {
                //Set numerical fields
                this.m_Instr = meta.instr;
                this.m_InstrBytes = meta.instr_bytes;
                this.m_Args = meta.args;
                this.m_ArgBytes = meta.arg_bytes;
                this.m_TotalBytes = meta.total_bytes;

                //Marshall strings
                this.m_Source = Marshal.PtrToStringAnsi(meta.source_process);
                this.m_Dest = Marshal.PtrToStringAnsi(meta.dest_process);
            }

            public String Source()
            {
                return this.m_Source;
            }

            public String Destination()
            {
                return this.m_Dest;
            }

            public UInt64 Instruction()
            {
                return this.m_Instr;
            }

            public IntPtr Arguments()
            {
                return this.m_Args;
            }

            public UInt64 Size()
            {
                return this.m_TotalBytes;
            }

            public Int32 ArgumentSize()
            {
                return this.m_ArgBytes;
            }

            public Int32 InstructionBytes()
            {
                return this.m_InstrBytes;
            }

            private String m_Source;
            private String m_Dest;
            private UInt64 m_Instr;
            private IntPtr m_Args;
            private UInt64 m_TotalBytes;
            private Int32 m_ArgBytes;
            private Int32 m_InstrBytes;        
        }

        public IntPtr m_Handle = (IntPtr)0;
        private bool m_IsHandled = false;

        public WimpInstructionNode(IntPtr handle)
        {
            m_Handle = handle;
        }

        ~WimpInstructionNode()
        {
            if (m_Handle != (IntPtr)0 && !m_IsHandled)
            {
                wimp_instr_node_free(m_Handle);
                m_Handle = (IntPtr)0;
            }
        }

        public bool IsNull()
        {
            return m_Handle == (IntPtr)0;
        }

        public WimpInstrMeta GetMeta()
        {
            if (m_Handle == (IntPtr)0)
            {
                return new WimpInstrMeta();
            }

            //Get the unmanaged data meta
            _WimpInstrMeta umeta = wimp_instr_get_from_node(m_Handle);
            return new WimpInstrMeta(umeta);
        }

        //Public but shouldn't be used by the user, only for internal use
        public IntPtr _Handle()
        {
            return m_Handle;
        }

        //For when handing back to another process, we don't want the node to be freed by the destructor
        public void MarkAsHandled()
        {
            m_IsHandled = true;
        }
    }

    public class WimpInstruction
    {
    }
}
