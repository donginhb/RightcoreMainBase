'\" te
.TH dhcp_network 4 "13 Mar 2001" "SunOS 5.8" "File Formats"
.SH "NAME"
dhcp_network \- DHCP network tables
.SH "DESCRIPTION"
.PP
The Dynamic Host Configuration Protocol (\fBDHCP\fR) network tables are used to map the client identifiers of \fBDHCP\fR clients to \fBIP\fR addresses and the associated configuration parameters of that address\&. One \fBDHCP\fR network table exists for each network served by the \fBDHCP\fR server, and each table is named using the network\&'s \fBIP\fR address\&. There is no table
or file with the name \fBdhcp_network\fR\&. 
.PP
The \fBDHCP\fR network tables can exist as \fBASCII\fR text files, binary text files, or \fBNIS+\fR tables, depending on the data store used\&. Since the format of the file could change, the preferred method of managing the \fBDHCP\fR
network tables is through the use of \fBdhcpmgr\fR(1M) or the \fBpntadm\fR(1M) command\&.
.PP
The format of the records in a \fBDHCP\fR network table depends on the data store used to maintain the table\&. However, an entry in a \fBDHCP\fR network table must contain the following fields:
.IP "\fBClient_ID\fR " 6
The client identifier field, \fBClient_ID\fR, is an \fBASCII\fR hexadecimal representation of the unique octet string  value of the  \fBDHCP\fR Client Identifier Option (code 61) which identifies a \fBDHCP\fR client\&. In the absence of the  \fBDHCP\fR Client Identifier Option, the  \fBDHCP\fR client is identified using the form given below for \fBBOOTP\fR clients\&. The number of characters in this field must be an even number, with a maximum length of 64 characters\&. Valid characters are \fB0\fR \fB-\fR \fB9\fR and \fBA\fR-\fBF\fR\&. Entries with values of \fB00\fR are freely available for dynamic allocation to requesting clients\&. \fBBOOTP\fR clients are identified by the concatenation of the network\&'s hardware type (as defined by \fBRFC\fR 1340, titled "Assigned Numbers") and the client\&'s hardware
address\&. For example, the following \fBBOOTP\fR client has a hardware type of \&'\fB01\fR\&' (10mb ethernet) and a hardware address of \fB8:0:20:11:12:b7\fR, so its client identifier would be: \fB010800201112B7\fR 
.IP "\fBFlags\fR " 6
The \fBFlags\fR field is a decimal value, the bit fields of which can have a combination of the following values:
.RS
.IP "\fB1 (PERMANENT)\fR " 6
Evaluation of the \fBLease\fR field is turned off (lease is permanent)\&. If this bit is not set,  Evaluation of the \fBLease\fR field is enabled and the  \fBLease\fR is  \fBDYNAMIC\&.\fR 
.IP "\fB2 (MANUAL)\fR " 6
This entry has a manual client \fBID\fR binding (cannot be reclaimed by \fBDHCP\fR server)\&.  Client will not be allocated another address\&.
.IP "\fB4 (UNUSABLE)\fR " 6
When set, this value means that either through \fBICMP\fR echo or client \fBDECLINE,\fR this address has been found to be unusable\&. Can also be used by the network administrator
to \fIprevent\fR a certain client from booting, if used in conjunction with the \fBMANUAL\fR flag\&.
.IP "\fB8 (BOOTP)\fR " 6
This entry is reserved for allocation to \fBBOOTP\fR clients only\&.
.sp
.RE
.IP "\fBClient_IP\fR " 6
The \fBClient_IP\fR field holds the \fBIP\fR address for this entry\&. This value must be unique in the database\&.
.IP "\fBServer_IP\fR " 6
This field holds the \fBIP\fR address of the \fBDHCP\fR server which \fIowns\fR this client \fBIP\fR address, and thus is responsible
for initial allocation to a requesting client\&.
.IP "\fBLease\fR " 6
This numeric field holds the entry\&'s absolute lease expiration time, and is in seconds since \fBJanuary 1, 1970\fR\&. It can be decimal, or hexadecimal (if \fB0x\fR prefixes number)\&. The special value \fB-1\fR is used to denote a permanent lease\&.
.IP "\fBMacro\fR " 6
This \fBASCII\fR text field contains the \fBdhcptab\fR macro name used to look up this entry\&'s configuration parameters in the \fBdhcptab\fR(4) database\&.
.IP "\fBComment\fR " 6
This \fBASCII\fR text field contains an optional comment\&.
.SS "TREATISE ON LEASES"
.PP
This section describes how the \fBDHCP/BOOTP\fR server calculates a client\&'s configuration lease using information contained in the \fBdhcptab\fR(4) and \fBDHCP\fR network tables\&. The server consults the \fBLeaseTim\fR and \fBLeaseNeg\fR symbols in the \fBdhcptab\fR, and the \fBFlags\fR and \fBLease\fR fields of the chosen IP address record in the \fBDHCP\fR network table\&.
.PP
The server first examines the \fBFlags\fR field for the identified \fBDHCP\fR network table record\&. If the \fBPERMANENT\fR flag is on, then the client\&'s lease is considered permanent\&.
.PP
If the \fBPERMANENT\fR flag is not on, the server checks if the client\&'s lease as represented by the \fBLease\fR field in the network table record has expired\&. If the lease is not expired, the server checks if the client has requested a new lease\&.
If the \fBLeaseNeg\fR symbol has not been included in the client\&'s \fBdhcptab\fR parameters, then the client\&'s requested lease extension is ignored, and the lease is set to be the time remaining as shown by the \fBLease\fR field\&. If the \fBLeaseNeg\fR
symbol \fIhas\fR been included, then the server will extend the client\&'s lease to the value it requested if this requested lease is less than or equal to the current time plus the value of the client\&'s \fBLeaseTim\fR \fBdhcptab\fR parameter\&.
.PP
If the client\&'s requested lease is greater than policy allows (value of \fBLeaseTim\fR), then the client is given a lease equal to the current time plus the value of \fBLeaseTim\fR\&. If \fBLeaseTim\fR is not set, then the default \fBLeaseTim\fR
value is one hour\&.
.PP
For more information about the \fBdhcptab\fR symbols, see \fBdhcptab\fR(4)\&. 
.SH "SEE ALSO"
.PP
\fBdhcpconfig\fR(1M), \fBdhcpmgr\fR(1M), \fBdhtadm\fR(1M), \fBin\&.dhcpd\fR(1M), \fBpntadm\fR(1M), \fBdhcptab\fR(4), \fBdhcp\fR(5), \fBdhcp_modules\fR(5)
.PP
\fISolaris DHCP Service Developer\&'s Guide\fR
.PP
\fISystem Administration Guide, Volume 3\fR
.PP
Reynolds, J\&. and J\&. Postel, \fIAssigned Numbers\fR, STD 2, RFC 1340, USC/Information Sciences Institute, July 1992\&.
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 14:04
