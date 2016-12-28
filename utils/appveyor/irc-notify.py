"""
From: https://raw.githubusercontent.com/nexB/scancode-toolkit/48aeaf76ce9f53d02223c41c1b2ad1d1ad73b851/etc/scripts/irc-notify.py

Copyright (C) 2015-2016 Christopher R. Wood

This program is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street,
Fifth Floor, Boston, MA 02110-1301 USA.

Simple AppVeyor IRC notification script.

Modified by nexB on October 2016:
 - rework the handling of environment variables.
 - made the script use functions
 - support only Appveyor loading its environment variable to craft IRC notices.

Modified by Jyrki Vesterinen on November 2016:

 - join the channel instead of sending an external message.

The first argument is a Frenode channel. Other arguments passed to the script will be
sent as notice messages content and any {var}-formatted environment variables will be
expanded automatically, replaced with a corresponding Appveyor environment variable
value. se commas to delineate multiple messages.


Example:
export APPVEYOR_URL=https://ci.appveyor.com
export APPVEYOR_PROJECT_NAME=attributecode
export APPVEYOR_REPO_COMMIT_AUTHOR=pombredanne
export APPVEYOR_REPO_COMMIT_TIMESTAMP=2016-10-31
export APPVEYOR_REPO_PROVIDER=gihub
export APPVEYOR_REPO_BRANCH=repo_branch
export APPVEYOR_PULL_REQUEST_TITLE=pull_request_title
export APPVEYOR_BUILD_VERSION=1
export APPVEYOR_REPO_COMMIT=22c95b72e29248dc4de9b85e590ee18f6f587de8
export APPVEYOR_REPO_COMMIT_MESSAGE="some IRC test"
export APPVEYOR_ACCOUNT_NAME=nexB
export APPVEYOR_PULL_REQUEST_NUMBER=pull_request_number
export APPVEYOR_REPO_NAME=nexB/attributecode
python etc/scripts/irc-notify.py aboutcode '[{project_name}:{branch}] {short_commit}: "{message}" ({author}) {color_red}Succeeded','Details: {build_url} | Commit: {commit_url}'

See also https://github.com/gridsync/gridsync/blob/master/appveyor.yml for examples
in Appveyor's YAML:

    on_success:
      - "python etc/scripts/irc-notify.py channel [{project_name}:{branch}] {short_commit}: \"{message}\" ({author}) {color_green}Succeeded,Details: {build_url},Commit: {commit_url}"
    on_failure:
      - "python etc/scripts/irc-notify.py channel [{project_name}:{branch}] {short_commit}: \"{message}\" ({author}) {color_red}Failed,Details: {build_url},Commit: {commit_url}"

"""

import os, random, socket, ssl, sys, time


def appveyor_vars():
    """
    Return a dict of key value carfted from appveyor environment variables.
    """
    from os import environ

    appveyor_url = environ.get('APPVEYOR_URL')
    message_extended = environ.get('APPVEYOR_REPO_COMMIT_MESSAGE_EXTENDED')
    configuration_name = environ.get('CONFIGURATION')
    branch = environ.get('APPVEYOR_REPO_BRANCH')
    author = environ.get('APPVEYOR_REPO_COMMIT_AUTHOR')
    author_email = environ.get('APPVEYOR_REPO_COMMIT_AUTHOR_EMAIL')
    timestamp = environ.get('APPVEYOR_REPO_COMMIT_TIMESTAMP')
    repo_provider = environ.get('APPVEYOR_REPO_PROVIDER')
    project_name = environ.get('APPVEYOR_PROJECT_NAME')
    project_slug = environ.get('APPVEYOR_PROJECT_SLUG')
    pull_request_title = environ.get('APPVEYOR_PULL_REQUEST_TITLE')
    build_version = environ.get('APPVEYOR_BUILD_VERSION')
    commit = environ.get('APPVEYOR_REPO_COMMIT')
    message = environ.get('APPVEYOR_REPO_COMMIT_MESSAGE')
    account_name = environ.get('APPVEYOR_ACCOUNT_NAME')
    pull_request_number = environ.get('APPVEYOR_PULL_REQUEST_NUMBER')
    repo_name = environ.get('APPVEYOR_REPO_NAME')

    short_commit = commit[:7]
    build_url = '{appveyor_url}/project/{account_name}/{project_slug}/build/{build_version}'.format(**locals())
    commit_url = 'https://{repo_provider}.com/{repo_name}/commit/{commit}'.format(**locals())

    vars = dict(
        appveyor_url=appveyor_url,
        account_name=account_name,
        project_name=project_name,
        project_slug=project_slug,
        build_version=build_version,

        build_url=build_url,

        repo_provider=repo_provider,
        repo_name=repo_name,
        branch=branch,
        configuration_name=configuration_name,
        author=author,
        author_email=author_email,
        timestamp=timestamp,
        commit=commit,
        short_commit=short_commit,
        message=message,
        message_extended=message_extended,

        pull_request_title=pull_request_title,
        pull_request_number=pull_request_number,

        commit_url=commit_url,

        color_green='\x033',
        color_red='\x034',
        bold='\x02',
        underline='\x1f',
        plain='\x0f',
    )
    return vars


if __name__ == '__main__':
    apvy_vars = appveyor_vars()

    channel = sys.argv[1]
    messages = sys.argv[2:]
    messages = ' '.join(messages)
    messages = messages.split(',')
    messages = [msg.format(**apvy_vars).strip() for msg in messages]

    irc_username = 'Appveyor'
    irc_nick = irc_username + str(random.randint(1, 9999))

    try:
        # establish connection
        irc_sock = ssl.wrap_socket(socket.socket(socket.AF_INET, socket.SOCK_STREAM))
        irc_sock.connect((socket.gethostbyname('chat.freenode.net'), 6697))
        irc_sock.send('NICK {0}\r\nUSER {0} * 0 :{0}\r\n'.format(irc_username).encode('utf_8'))
        irc_file = irc_sock.makefile()

        while irc_file:
            line = irc_file.readline()
            print(line.rstrip())
            response = line.split()

            if response[0] == 'PING':
                irc_file.send('PONG {}\r\n'.format(reponse[1]).encode('utf_8'))

            elif response[1] == '433':
                irc_sock.send('NICK {}\r\n'.format(irc_nick).encode('utf_8'))

            elif response[1] == '001':
                time.sleep(5)
                # join the channel
                irc_sock.send('JOIN #{}\r\n'.format(channel).encode('utf_8'))
                # send messages
                for msg in messages:
                    print('PRIVMSG #{} :{}'.format(channel, msg))
                    irc_sock.send('PRIVMSG #{} :{}\r\n'.format(channel, msg).encode('utf_8'))
                time.sleep(5)
                # leave the channel
                irc_sock.send('PART #{}\r\n'.format(channel).encode('utf_8'))
                sys.exit()
    except:
        e = sys.exc_info()[0]
        print(e)
        sys.exit()
