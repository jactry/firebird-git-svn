/*
 * The contents of this file are subject to the Interbase Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy
 * of the License at http://www.Inprise.com/IPL.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code was created by Inprise Corporation
 * and its predecessors. Portions created by Inprise Corporation are
 * Copyright (C) Inprise Corporation.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 */
#define	SYI$C_EXETYPE	1
#define	SYI$C_FLDTYPE	2
#define	SYI$C_RSDTYPE	3
#define	SYI$_VERSION	4096
#define	SYI$_SID	4097
#define	SYI$_PFCDEFAULT	4098
#define	SYI$_PAGTBLPFC	4099
#define	SYI$_SYSPFC	4100
#define	SYI$_KFILSTCNT	4101
#define	SYI$_GBLSECTIONS	4102
#define	SYI$_GBLPAGES	4103
#define	SYI$_GBLPAGFIL	4104
#define	SYI$_MAXPROCESSCNT	4105
#define	SYI$_PIXSCAN	4106
#define	SYI$_PROCSECTCNT	4107
#define	SYI$_MINWSCNT	4108
#define	SYI$_PAGFILCNT	4109
#define	SYI$_SWPFILCNT	4110
#define	SYI$_SYSMWCNT	4111
#define	SYI$_INTSTKPAGES	4112
#define	SYI$_DLCKEXTRASTK	4113
#define	SYI$_BALSETCNT	4114
#define	SYI$_IRPCOUNT	4115
#define	SYI$_IRPCOUNTV	4116
#define	SYI$_WSMAX	4117
#define	SYI$_NPAGEDYN	4118
#define	SYI$_NPAGEVIR	4119
#define	SYI$_PAGEDYN	4120
#define	SYI$_VIRTUALPAGECNT	4121
#define	SYI$_SPTREQ	4122
#define	SYI$_EXUSRSTK	4123
#define	SYI$_LRPCOUNT	4124
#define	SYI$_LRPCOUNTV	4125
#define	SYI$_LRPSIZE	4126
#define	SYI$_LRPMIN	4127
#define	SYI$_SRPCOUNT	4128
#define	SYI$_SRPCOUNTV	4129
#define	SYI$_SRPSIZE	4130
#define	SYI$_SRPMIN	4131
#define	SYI$_CHANNELCNT	4132
#define	SYI$_PIOPAGES	4133
#define	SYI$_CTLPAGES	4134
#define	SYI$_CTLIMGLIM	4135
#define	SYI$_IMGIOCNT	4136
#define	SYI$_QUANTUM	4137
#define	SYI$_MPW_WRTCLUSTER	4138
#define	SYI$_MPW_HILIMIT	4139
#define	SYI$_MPW_LOLIMIT	4140
#define	SYI$_MPW_PRIO	4141
#define	SYI$_SWP_PRIO	4142
#define	SYI$_MPW_THRESH	4143
#define	SYI$_MPW_WAITLIMIT	4144
#define	SYI$_TBSKIPWSL	4145
#define	SYI$_PHYSICALPAGES	4146
#define	SYI$_PFRATL	4147
#define	SYI$_PFRATH	4148
#define	SYI$_PFRATS	4149
#define	SYI$_WSINC	4150
#define	SYI$_WSDEC	4151
#define	SYI$_AWSMIN	4152
#define	SYI$_AWSTIME	4153
#define	SYI$_SWPRATE	4154
#define	SYI$_SWPOUTPGCNT	4155
#define	SYI$_SWPALLOCINC	4156
#define	SYI$_IOTA	4157
#define	SYI$_LONGWAIT	4158
#define	SYI$_SWPFAIL	4159
#define	SYI$_VMSD1	4160
#define	SYI$_VMSD2	4161
#define	SYI$_VMSD3	4162
#define	SYI$_VMSD4	4163
#define	SYI$_VMS5	4164
#define	SYI$_VMS6	4165
#define	SYI$_VMS7	4166
#define	SYI$_VMS8	4167
#define	SYI$_USERD1	4168
#define	SYI$_USERD2	4169
#define	SYI$_USER3	4170
#define	SYI$_USER4	4171
#define	SYI$_EXTRACPU	4172
#define	SYI$_MAXSYSGROUP	4173
#define	SYI$_MVTIMEOUT	4174
#define	SYI$_MAXBUF	4175
#define	SYI$_DEFMBXBUFQUO	4176
#define	SYI$_DEFMBXMXMSG	4177
#define	SYI$_DEFMBXNUMMSG	4178
#define	SYI$_FREELIM	4179
#define	SYI$_FREEGOAL	4180
#define	SYI$_GROWLIM	4181
#define	SYI$_BORROWLIM	4182
#define	SYI$_LOCKRETRY	4183
#define	SYI$_XFMAXRATE	4184
#define	SYI$_LAMAPREGS	4185
#define	SYI$_REALTIME_SPTS	4186
#define	SYI$_CLISYMTBL	4187
#define	SYI$_LOCKIDTBL	4188
#define	SYI$_RESHASHTBL	4189
#define	SYI$_DEADLOCK_WAIT	4190
#define	SYI$_SCSBUFFCNT	4191
#define	SYI$_SCSCONNCNT	4192
#define	SYI$_SCSRESPCNT	4193
#define	SYI$_SCSMAXDG	4194
#define	SYI$_SCSMAXMSG	4195
#define	SYI$_SCSFLOWCUSH	4196
#define	SYI$_SCSSYSTEMID	4197
#define	SYI$_SCSSYSTEMIDH	4198
#define	SYI$_SCSNODE	4199
#define	SYI$_PRCPOLINTERVAL	4201
#define	SYI$_PASTIMOUT	4202
#define	SYI$_PASTDGBUF	4203
#define	SYI$_PANUMPOLL	4204
#define	SYI$_PAPOLLINTERVAL	4205
#define	SYI$_PAPOOLINTERVAL	4206
#define	SYI$_TIMEPROMPTWAIT	4207
#define	SYI$_UDABURSTRATE	4208
#define	SYI$_LNMSHASHTBL	4209
#define	SYI$_LNMPHASHTBL	4210
#define	SYI$_TTY_SCANDELTA	4211
#define	SYI$_TTY_DIALTYPE	4212
#define	SYI$_TTY_SPEED	4213
#define	SYI$_TTY_RSPEED	4214
#define	SYI$_TTY_PARITY	4215
#define	SYI$_TTY_BUF	4216
#define	SYI$_TTY_DEFCHAR	4217
#define	SYI$_TTY_DEFCHAR2	4218
#define	SYI$_TTY_TYPAHDSZ	4219
#define	SYI$_TTY_ALTYPAHD	4220
#define	SYI$_TTY_ALTALARM	4221
#define	SYI$_TTY_DMASIZE	4222
#define	SYI$_TTY_PROT	4223
#define	SYI$_TTY_OWNER	4224
#define	SYI$_TTY_CLASSNAME	4225
#define	SYI$_TTY_SILOTIME	4226
#define	SYI$_TTY_DEFPORT	4227
#define	SYI$_RMS_DFMBC	4228
#define	SYI$_RMS_DFMBFSDK	4229
#define	SYI$_RMS_DFMBFSMT	4230
#define	SYI$_RMS_DFMBFSUR	4231
#define	SYI$_RMS_DFMBFREL	4232
#define	SYI$_RMS_DFMBFIDX	4233
#define	SYI$_RMS_DFMBFHSH	4234
#define	SYI$_RMS_PROLOGUE	4235
#define	SYI$_RMS_EXTEND_SIZE	4236
#define	SYI$_RMS_FILEPROT	4237
#define	SYI$_PQL_DASTLM	4238
#define	SYI$_PQL_MASTLM	4239
#define	SYI$_PQL_DBIOLM	4240
#define	SYI$_PQL_MBIOLM	4241
#define	SYI$_PQL_DBYTLM	4242
#define	SYI$_PQL_MBYTLM	4243
#define	SYI$_PQL_DCPULM	4244
#define	SYI$_PQL_MCPULM	4245
#define	SYI$_PQL_DDIOLM	4246
#define	SYI$_PQL_MDIOLM	4247
#define	SYI$_PQL_DFILLM	4248
#define	SYI$_PQL_MFILLM	4249
#define	SYI$_PQL_DPGFLQUOTA	4250
#define	SYI$_PQL_MPGFLQUOTA	4251
#define	SYI$_PQL_DPRCLM	4252
#define	SYI$_PQL_MPRCLM	4253
#define	SYI$_PQL_DTQELM	4254
#define	SYI$_PQL_MTQELM	4255
#define	SYI$_PQL_DWSDEFAULT	4256
#define	SYI$_PQL_MWSDEFAULT	4257
#define	SYI$_PQL_DWSQUOTA	4258
#define	SYI$_PQL_MWSQUOTA	4259
#define	SYI$_PQL_DWSEXTENT	4260
#define	SYI$_PQL_MWSEXTENT	4261
#define	SYI$_PQL_DENQLM	4262
#define	SYI$_PQL_MENQLM	4263
#define	SYI$_ACP_MAPCACHE	4264
#define	SYI$_ACP_HDRCACHE	4265
#define	SYI$_ACP_DIRCACHE	4266
#define	SYI$_ACP_WORKSET	4267
#define	SYI$_ACP_FIDCACHE	4268
#define	SYI$_ACP_EXTCACHE	4269
#define	SYI$_ACP_EXTLIMIT	4270
#define	SYI$_ACP_QUOCACHE	4271
#define	SYI$_ACP_SYSACC	4272
#define	SYI$_ACP_MAXREAD	4273
#define	SYI$_ACP_WINDOW	4274
#define	SYI$_ACP_WRITEBACK	4275
#define	SYI$_ACP_DATACHECK	4276
#define	SYI$_ACP_BASEPRIO	4277
#define	SYI$_ACP_SWAPFLGS	4278
#define	SYI$_DEFPRI	4279
#define	SYI$_IJOBLIM	4280
#define	SYI$_BJOBLIM	4281
#define	SYI$_NJOBLIM	4282
#define	SYI$_RJOBLIM	4283
#define	SYI$_QUORUM	4284
#define	SYI$_VOTES	4285
#define	SYI$_RECNXINTERVAL	4286
#define	SYI$_BOOTTIME	4287
#define	SYI$_LOCKIDTBL_MAX	4288
#define	SYI$_TAILORED	4289
#define	SYI$_STARTUP_P1	4290
#define	SYI$_STARTUP_P2	4291
#define	SYI$_STARTUP_P3	4292
#define	SYI$_STARTUP_P4	4293
#define	SYI$_STARTUP_P5	4294
#define	SYI$_STARTUP_P6	4295
#define	SYI$_STARTUP_P7	4296
#define	SYI$_STARTUP_P8	4297
#define	SYI$_CLUSTER_NODES	4298
#define	SYI$_CLUSTER_VOTES	4299
#define	SYI$_CLUSTER_QUORUM	4300
#define	SYI$_CLUSTER_FSYSID	4301
#define	SYI$_CLUSTER_FTIME	4302
#define	SYI$_CLUSTER_MEMBER	4303
#define	SYI$_NODE_CSID	4304
#define	SYI$_NODE_VOTES	4305
#define	SYI$_NODE_QUORUM	4306
#define	SYI$_NODE_SYSTEMID	4307
#define	SYI$_NODE_SWINCARN	4308
#define	SYI$_NODE_SWTYPE	4309
#define	SYI$_NODE_SWVERS	4310
#define	SYI$_NODE_HWTYPE	4311
#define	SYI$_NODE_HWVERS	4312
#define	SYI$_NODENAME	4313
#define	SYI$_ARCHFLAG	4314
#define	SYI$_SCS_EXISTS	4315
#define	SYI$_DISK_QUORUM	4316
#define	SYI$_PAMAXPORT	4320
#define	SYI$_PASANITY	4321
#define	SYI$_DEFQUEPRI	4322
#define	SYI$_MAXQUEPRI	4323
#define	SYI$_QDSKINTERVAL	4324
#define	SYI$_ALLOCLASS	4325
#define	SYI$_LGI_RETRY_LIM	4326
#define	SYI$_LGI_RETRY_TMO	4327
#define	SYI$_LGI_BRK_LIM	4328
#define	SYI$_LGI_BRK_TMO	4329
#define	SYI$_LGI_HID_TIM	4330
#define	SYI$_LGI_PWD_TMO	4331
#define	SYI$_PQL_DJTQUOTA	4332
#define	SYI$_PQL_MJTQUOTA	4333
#define	SYI$_VAXCLUSTER	4334
#define	SYI$_LOCKDIRWT	4335
#define	SYI$_QDSKVOTES	4336
#define	SYI$_DORMANTWAIT	4337
#define	SYI$_PAGEFILE_PAGE	4338
#define	SYI$_SWAPFILE_PAGE	4339
#define	SYI$_PAGEFILE_FREE	4340
#define	SYI$_SWAPFILE_FREE	4341
#define	SYI$_TTY_TIMEOUT	4342
#define	SYI$_TTY_AUTOCHAR	4343
#define	SYI$_PANOPOLL	4344
#define	SYI$_PE1	4345
#define	SYI$_PE2	4346
#define	SYI$_PE3	4347
#define	SYI$_PE4	4348
#define	SYI$_PE5	4349
#define	SYI$_PE6	4350
#define	SYI$_RMS_GBLBUFQUO	4351
#define	SYI$_RMS_DFNBC	4352
#define	SYI$_ACP_DINDXCACHE	4353
#define	SYI$_LASTEXE	4354
#define	SYI$_CPU	8192
#define	SYI$_BUGREBOOT	8193
#define	SYI$_CRDENABLE	8194
#define	SYI$_DUMPBUG	8195
#define	SYI$_BUGCHECKFATAL	8196
#define	SYI$_ACP_MULTIPLE	8197
#define	SYI$_NOAUTOCONFIG	8198
#define	SYI$_NOCLOCK	8199
#define	SYI$_NOCLUSTER	8200
#define	SYI$_POOLPAGING	8201
#define	SYI$_SBIERRENABLE	8202
#define	SYI$_SETTIME	8203
#define	SYI$_ACP_SHARE	8204
#define	SYI$_SYSPAGING	8205
#define	SYI$_UAFALTERNATE	8206
#define	SYI$_WRITABLESYS	8207
#define	SYI$_RESALLOC	8208
#define	SYI$_SSINHIBIT	8209
#define	SYI$_CONCEAL_DEVICES	8210
#define	SYI$_SAVEDUMP	8211
#define	SYI$_MOUNTMSG	8212
#define	SYI$_DISMOUMSG	8213
#define	SYI$_LOADERAPT	8214
#define	SYI$_LOADCHKPRT	8215
#define	SYI$_CJFLOAD	8217
#define	SYI$_CJFSYSRUJ	8218
#define	SYI$_NODE_AREA	8219
#define	SYI$_NODE_NUMBER	8220
#define	SYI$_CLASS_PROT	8221
#define	SYI$_CHARACTER_EMULATED	8222
#define	SYI$_DECIMAL_EMULATED	8223
#define	SYI$_D_FLOAT_EMULATED	8224
#define	SYI$_F_FLOAT_EMULATED	8225
#define	SYI$_G_FLOAT_EMULATED	8226
#define	SYI$_H_FLOAT_EMULATED	8227
#define	SYI$_LOADMTACCESS	8228
#define	SYI$_ACP_XQP_RES	8229
#define	SYI$_WRITESYSPARAMS	8230
#define	SYI$_LGI_BRK_TERM	8231
#define	SYI$_LGI_BRK_DISUSER	8232
#define	SYI$_ACP_REBLDSYSD	8233
#define	SYI$_WS_OPA0	8234
#define	SYI$_LASTFLD	8235
#define	SYI$C_SFWTYPE	1
#define	SYI$C_HDWTYPE	2
#define	SYI$C_LISTEND	0
#define	SYI$_OLDVERSION	256
#define	SYI$_LASTSFW	257
#define	SYI$_OLDCPU	512
#define	SYI$_OLDSID	513
#define	SYI$_LASTHDW	514
