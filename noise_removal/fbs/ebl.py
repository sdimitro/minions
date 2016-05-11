
from bs4 import BeautifulSoup

import sys

blacklist = [
    "place:folder=BOOKMARKS_MENU&folder=UNFILED_BOOKMARKS&folder=TOOLBAR&queryType=1&sort=12&maxResults=10&excludeQueries=1",
    "place:type=6&sort=14&maxResults=10",
    "http://www.ubuntu.com/",
    "http://wiki.ubuntu.com/",
    "https://answers.launchpad.net/ubuntu/+addquestion",
    "http://www.debian.org/",
    "https://one.ubuntu.com/",
    "https://www.mozilla.org/en-US/firefox/help/",
    "https://www.mozilla.org/en-US/firefox/customize/",
    "https://www.mozilla.org/en-US/contribute/",
    "https://www.mozilla.org/en-US/about/",
    "place:sort=8&maxResults=10",
    "https://www.mozilla.org/en-US/firefox/central/"
]


def extract_links(filename):
    soup = BeautifulSoup(open(filename))
    return [link.get('href') for link in soup.find_all('a')]


def filter(links, blacklisted):
    return list(set(links) - set(blacklisted))


def print_links(links):
    for l in links:
        print(l)


def main(filename):
    print_links(filter(extract_links(filename), blacklist))

main(sys.argv[1])
