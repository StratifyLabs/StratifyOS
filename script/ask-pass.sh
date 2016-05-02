#!/bin/bash
#If you add "sudo install" to the login keychain and then add your sudo password, this code will get it using /usr/bin security

platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
get_pw () {
  security 2>&1 >/dev/null find-generic-password -g -s "sudo install" | ruby -e 'print $1 if STDIN.gets =~ /^password: "(.*)"$/'
}
echo $(get_pw)
elif [[ "$unamestr" == 'Linux' ]]; then
	echo "YourPassword"
fi



