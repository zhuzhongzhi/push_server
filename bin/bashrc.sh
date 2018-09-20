# .bashrc

# Source global definitions
if [ -f ${HOME}/.bashrc ]; then
	. ${HOME}/.bashrc
fi

# User specific aliases and functions
export ACE_ROOT=/usr/local
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export WORK_DIR=`dirname $PWD`
export PATH=${WORK_DIR}/bin:$PATH
export PROC_NAME=push_server
export PROC_VER=version1.1
alias bin="cd ${WORK_DIR}/bin"
alias wk="cd ${WORK_DIR}"
alias src="cd ${WORK_DIR}/src"
alias cfg="cd ${WORK_DIR}/config"
alias log="cd ${WORK_DIR}/log"
alias rmlog="rm ${WORK_DIR}/log/*;rm ${WORK_DIR}/log/.*"
alias me="src;make"
alias ss="${WORK_DIR}/bin/im_server&"
alias st="${WORK_DIR}/bin/stop.sh"
