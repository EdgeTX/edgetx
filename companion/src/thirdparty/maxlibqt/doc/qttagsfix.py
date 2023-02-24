#!/usr/bin/env python
__copyright__ = """
COPYRIGHT: (c)2020 Maxim Paperno; All Rights Reserved.

Released under MIT License:

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

DESCRIPTION = """
A utility to fix Qt's Doxygen-compatible tag files which have invalid data 
for enum types and values, as per QTBUG-61790.  It can be used as a standalone
utility, or imported as a package to call the fixEnums() method directly.

It reads a given input file, applies the fixes, and writes the fixed XML
back to a file (either the same one as the input, or a new one). 

It looks for tags improperly formatted like this:

  <member kind="enum">
    <name>Orientation</name>
    <anchor>Orientation-enum</anchor>
    <arglist></arglist>
  </member>
  <member name="Horizontal">
    <anchor>Orientation-enum</anchor>
    <arglist></arglist>
  </member>

And converts them to:

  <member kind="enumeration">
    <name>Orientation</name>
    <anchorfile>qt.html</anchorfile>
    <anchor>Orientation-enum</anchor>
  </member>
  <member kind="enumvalue">
    <name>Horizontal</name>
    <anchorfile>qt.html</anchorfile>
    <anchor>Orientation-enum</anchor>
  </member>

One limitation is that there is no way to detect deprecated enums, so those
will still be linked to the parent's main docs page, instead of the "-obsolete"
page where they actually are listed.
"""

import sys
import argparse
import xml.etree.ElementTree as ET

# used as parser target for preserving comments in XML
class CommentedTreeBuilder(ET.TreeBuilder):
  def comment(self, data):
    self.start(ET.Comment, {})
    self.data(data)
    self.end(ET.Comment)


# to strip useless arglist tag
def stripEmptyArglist(parent):
  alist = parent.find('arglist')
  if alist is not None and not alist.text:
    parent.remove(alist)
    lastEl = parent.find('anchor')
    if lastEl is not None:
      parent.find('anchor').tail = parent.tail


# Fix enum members in Qt tagfiles (QTBUG-61790).
# `infile` is the full path to the XML tag file to process.
# If `outfile` is `None` (default) then the `infile` file is overwritten.
def fixEnums(infile, outfile=None):
  if not outfile:
    outfile = infile
  tree = ET.parse(infile, ET.XMLParser(target=CommentedTreeBuilder()))
  root = tree.getroot()

  # iterate over all compounds with named enum members
  for comp in root.iterfind(".//member[@kind='enum']/name/../.."):
    if comp is None:
      continue
    # the file name of the parent compound becomes the <anchorfile> for enum links
    fn = comp.find('filename')
    if fn is None:
      continue
    # Skip the whole compound if it doesn't have the broken enum syntax anywhere
    # (we could probably just bail out entirely here, but who knows...)
    if comp.find(".//member[@name]") is None:
      continue

    # get line break and indent level for inserting elements
    tail = comp.find("./member/name").tail
    # <anchorfile> element needed for enumeration and enumvalue tags
    afileEl = ET.Element('anchorfile')
    afileEl.text = fn.text
    afileEl.tail = tail

    # fix <member kind="enum"> tags (change to kind="enumeration" and add <anchorfile>)
    for enum in comp.iterfind(".//member[@kind='enum']"):
      if enum.find('anchorfile') is not None:
        continue  # has anchorfile element, must have been fixed already
      enum.set('kind', "enumeration")
      enum.insert(1, afileEl)
      stripEmptyArglist(enum)

    # fix enum value members which appear as <member name="ValueName"> but should
    # be <member kind="enumvalue" and have child <name> add <anchorfile> elements
    for enumval in comp.iterfind(".//member[@name]"):
      ename = enumval.get('name')
      if ename is None or enumval.find('name') is not None:
        continue  # must have been fixed already
      enumval.attrib.clear()  # remove 'name' attrib
      enumval.set('kind', "enumvalue")
      nameEl = ET.Element('name')
      nameEl.text = ename
      nameEl.tail = tail
      enumval.insert(0, nameEl)
      enumval.insert(1, afileEl)
      stripEmptyArglist(enumval)

  # write out the new XML
  tree.write(outfile, encoding="UTF-8", xml_declaration=True)


def main():
  parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=DESCRIPTION)

  parser.add_argument("infile",
    help="Input XML tagfile to fix.")
  parser.add_argument("outfile",
    help="Output file for fixed XML. Use 'overwrite' to modify the input file itself.")

  opts = parser.parse_args();

  if opts.outfile == "overwrite":
    opts.outfile = opts.infile

  print("Fixing up tagfile: " + opts.infile)
  print("Output to: " + opts.outfile)

  fixEnums(opts.infile, opts.outfile)

  return 0


if __name__ == "__main__":
  sys.exit(main())
