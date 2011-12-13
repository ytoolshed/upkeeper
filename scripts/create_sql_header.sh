#!/bin/sh
builddir="$1"
header="${builddir}/controller/ctrl_sql.h"
srcdir="${builddir}/controller/"

cat > "${header}.tmp" <<'EOT'
/****************************************************************************
* Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
* Apache License, Version 2.0 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of the License
* at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
* law or agreed to in writing, software distributed under the License is
* distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied. See the License for the specific language
* governing permissions and limitations under the License.
* See accompanying LICENSE file. 
************************************************************************** */

#ifndef _UPK_SQL_H
#define _UPK_SQL_H

/* *INDENT-OFF* */
EOT

for file in "$srcdir"/*.sql; do 
	varname=`basename "$file" | sed -e 's/\./_/g'`
	echo "const char ${varname}[] = " >> "${header}.tmp"
	OLDIFS="$IFS"
	IFS=`echo`
	while read line; do
		line=`echo "$line" | sed -e 's/"/\\\"/g'`
		echo "		\"$line\\n\"" >> "${header}.tmp"
	done < "$file"
	IFS="$OLDIFS"
	echo ';' >> "${header}.tmp"
done
echo "/* *INDENT-ON* */" >> "${header}.tmp"
echo "#endif" >> "${header}.tmp"
mv "${header}.tmp" "$header"
