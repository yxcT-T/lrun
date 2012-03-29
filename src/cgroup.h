////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 WU Jun <quark@zju.edu.cn>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <map>
#include <list>
#include <sys/resource.h>

namespace lrun {
    class Cgroup {
        public:

            // Cgroup static methods

            /**
             * get cgroup mounted path
             * @param   create_on_need  mount cgroup if not mounted
             * @return  cgroup mounted path (first one in mount table)
             */
            static std::string base_path(bool create_on_need = true);

            /**
             * create a cgroup, use existed if possible
             * @return 
             */
            static Cgroup create(const std::string& name);

            /**
             * @return  1           exist
             *          0           not exist
             */
            static int exists(const std::string& name);

            /**
             * @param   name        group name
             * @return  full path   "#{path_}/#{name}"
             */
            static std::string path_from_name(const std::string& name);


            // Cgroup low level methods

            /**
             * killall processes and destroy this cgroup
             * @return -1           failed
             *          0           success
             */
            int destroy();

            /**
             * set a cgroup property
             * @param   property    property
             * @param   value       value
             * @return  0           success
             *         <0           failed
             */
            int set(const std::string& property, const std::string& value);

            /**
             * get property
             * @param   property    property
             * @param   max_length  max length to read (not include '\0')
             * @return  string      readed property, empty if fail
             */
            std::string get(const std::string& property, size_t max_length = 255);

            /**
             * set a cgroup property to the same value as parent
             * @param   property    property
             * @return  0           success
             *         <0           failed
             */
            int inherit(const std::string& property);

            /**
             * attach a process
             * @param   pid         process id to attach
             * @return  0           success
             *         <0           failed
             */
            int attach(pid_t pid);

            /**
             * check if Cgroup is invalid
             * @return  true        valid
             *          false       invalid
             */
            bool valid();

            // Cgroup high level methods

            /**
             * kill all tasks until no more tasks
             * @return -1           error
             *          0           no tasks running
             *         >0           number of tasks killed
             */
            int killall();

            /**
             * get memory usage
             * @return  memory usage in bytes
             */
            long long memory_usage();

            /**
             * get memory limit
             * @return  memory limit in bytes
             */
            long long memory_limit();

            /**
             * get cpu usage
             * @return  cpu usage in seconds
             */
            double cpu_usage();

            /**
             * set memory usage limit
             * @param   bytes       limit, no limit if bytes <= 0
             * @return  0           success
             *         <0           failed
             */
            int set_memory_limit(long long bytes);

            /**
             * restart cpuacct and memory max_usage_in_bytes
             * @return  0           success
             *         <0           failed
             */
            int reset_usages();

            /**
             * limit devices to null, zero, full, random and urandom
             *
             * @return  0           success
             *         <0           failed
             */
            int limit_devices();

            /**
             * structure used for forked child
             */
            struct spawn_arg {
                int clone_flags;            // additional clone flags
                char * const * args;        // exec args
                uid_t uid;                  // uid (should not be 0)
                gid_t gid;                  // gid (should not be 0)
                int nice;                   // nice
                int sockets[2];             // for sync between child and parent
                std::string chroot_path;    // chroot path, empty if not need to chroot
                std::list<std::pair<std::string, long long> > tmpfs_list;
                                            // [(dest, bytes)] mount tmpfs in child FS (after chroot)
                std::list<std::pair<std::string, std::string> > bindfs_list; 
                                            // [(dest, src)] mount bind in child FS (before chroot)
                std::map<int, rlim_t> rlimits;
                                            // [resource, value] rlimit list
                int reset_env;              // Do not inherit env
                std::list<std::pair<std::string, std::string> > env_list;
                                            // environment variables whitelist
            };

            /**
             * spawn child process and exec inside cgroup
             * child process is in other namespace in FS, PID, UTS, IPC, NET
             * child process is attached to cgroup just before exec
             * @param   arg         swapn arg, @see struct spawn_arg
             * @return  pid         child pid, negative if failed
             */
            pid_t spawn(spawn_arg& arg);

        private:

            Cgroup();
            std::string path_;
    };
}
