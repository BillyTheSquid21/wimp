using System;
using System.Runtime.InteropServices;

namespace WimpCS
{
    /// 
    /// @brief Wraps the C bindings into C# code 
    ///
    public class Wimp
    {
        public enum WIMPInstructionsCore : UInt64
        { 
            EXIT = 1540385,
            LOG = 161821,
            PING = 1446181,
        }

        public enum WimpDataResult
        { 
            SUCCESS = 0,
            FAIL = -1,
        }

        public enum WimpInstructionResult
        {
            SUCCESS = 0,
            FAIL = -1,
        }

        public enum WimpProcessResult
        {
            SUCCESS = 0,
            FAIL = -1,
            INVALID_PATH = -2,
            UNRESOLVED_EXE_DIR = -3,
        }

        public enum WimpProcessTableResult
        {
            SUCCESS = 0,
            FAIL = -1,
        }

        public enum WimpRecieverResult
        {
            SUCCESS = 0,
            FAIL = -1,
        }

        public enum WimpServerResult
        {
            SUCCESS = 0,
            FAIL = -1,
            ADDRESS_FAIL = -2,
            SOCKET_FAIL = -3,
            BIND_FAIL = -4,
            LISTEN_FAIL = -5,
            TOO_FEW_PROCESSES = -6,
            UNEXPECTED_PROCESS = -7
        }

        public enum WimpRelation
        {
            Unknown = 0,
            Child = 1,
            Parent = 2,
            Independent = 3
        }

        [DllImport("wimp.dll")]
        public static extern void wimp_add_local_server([MarshalAs(UnmanagedType.LPStr)] String dest, UInt64 instr, IntPtr args, UInt64 arg_size_bytes);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_assign_unused_local_port();

        [DllImport("wimp.dll")]
        public static extern void wimp_close_local_server();

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_data_access(IntPtr arena, [MarshalAs(UnmanagedType.LPStr)] String name);

        [DllImport("wimp.dll")]
        public static extern void wimp_data_free();

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_data_init([MarshalAs(UnmanagedType.LPStr)] String memory_name);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_data_link_to_data([MarshalAs(UnmanagedType.LPStr)] String name);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_data_link_to_process([MarshalAs(UnmanagedType.LPStr)] String memory_name);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_data_reserve([MarshalAs(UnmanagedType.LPStr)] String reserved_name, UInt64 size);

        [DllImport("wimp.dll")]
        public static extern void wimp_data_stop_access(IntPtr arena, [MarshalAs(UnmanagedType.LPStr)] String name);

        [DllImport("wimp.dll")]
        public static extern void wimp_data_unlink_from_data([MarshalAs(UnmanagedType.LPStr)] String name);

        [DllImport("wimp.dll")]
        public static extern void wimp_data_unlink_from_process();

        [DllImport("wimp.dll")]
        public static extern void wimp_free_entry(IntPtr entry);

        [DllImport("wimp.dll")]
        public static extern IntPtr wimp_get_entry(Int32 argc, __arglist);

        [DllImport("wimp.dll")]
        public static extern IntPtr wimp_get_local_server();

        [DllImport("wimp.dll")]
        public static extern IntPtr wimp_get_reciever_args([MarshalAs(UnmanagedType.LPStr)] String process_name, [MarshalAs(UnmanagedType.LPStr)] String recfrom_domain, Int32 recfrom_port, IntPtr incomingq, IntPtr active);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_init();

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_init_local_server([MarshalAs(UnmanagedType.LPStr)] String process_name, [MarshalAs(UnmanagedType.LPStr)] String domain, Int32 port);

        [DllImport("wimp.dll")]
        public static extern UInt64 wimp_instr([MarshalAs(UnmanagedType.LPStr)] String text);

        [DllImport("wimp.dll")]
        public static extern bool wimp_instr_check(UInt64 instr1, UInt64 instr2);

        //[DllImport("wimp.dll")]
        //public static extern WimpInstrMeta wimp_instr_get_from_buffer(IntPtr buffer, UInt64 buffsize); - TODO find a portable alternative

        //[DllImport("wimp.dll")]
        //public static extern WimpInstrMeta wimp_instr_get_from_node(IntPtr node); - TODO find a portable alternative

        [DllImport("wimp.dll")]
        public static extern UInt64 wimp_instr_get_instruction_count(IntPtr queue, [MarshalAs(UnmanagedType.LPStr)] String instruction);

        [DllImport("wimp.dll")]
        public static extern void wimp_instr_node_free(IntPtr node);

        [DllImport("wimp.dll")]
        public static extern void wimp_instr_pack_free(IntPtr pack);

        [DllImport("wimp.dll")]
        public static extern IntPtr wimp_instr_pack_get_string(IntPtr pack, Int32 index);

        [DllImport("wimp.dll")]
        public static extern IntPtr wimp_instr_pack_strings(UInt64 count, __arglist);

        [DllImport("wimp.dll")]
        public static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, __arglist);

        [DllImport("wimp.dll")]
        public static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, __arglist);

        [DllImport("wimp.dll")]
        public static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, __arglist);

        [DllImport("wimp.dll")]
        public static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, __arglist);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_process_table_add(IntPtr table, [MarshalAs(UnmanagedType.LPStr)] String process_name, [MarshalAs(UnmanagedType.LPStr)] String process_domain, Int32 process_port, Int32 relation, IntPtr connection);

        [DllImport("wimp.dll")]
        public static extern bool wimp_server_check_process_listening(IntPtr server, [MarshalAs(UnmanagedType.LPStr)] String process_name);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_server_process_accept(IntPtr server, Int32 pcount, __arglist);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_server_send_instructions(IntPtr server);

        [DllImport("wimp.dll")]
        public static extern IntPtr wimp_server_wait_response(IntPtr server, UInt64 instr, Int32 timeout);

        [DllImport("wimp.dll")]
        public static extern void wimp_shutdown();

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_start_executable_process([MarshalAs(UnmanagedType.LPStr)] String process_name, [MarshalAs(UnmanagedType.LPStr)] String executable, IntPtr entry);

        [DllImport("wimp.dll")]
        public static extern Int32 wimp_start_reciever_thread([MarshalAs(UnmanagedType.LPStr)] String recfrom_name, [MarshalAs(UnmanagedType.LPStr)] String process_domain, Int32 process_port, IntPtr args);
    }
}