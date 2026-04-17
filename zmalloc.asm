<Z_Malloc>:
  4755ee:	55                   	push   %rbp
  4755ef:	48 89 e5             	mov    %rsp,%rbp
  4755f2:	48 81 ec 00 01 00 00 	sub    $0x100,%rsp
  4755f9:	4c 89 65 b8          	mov    %r12,-0x48(%rbp)
  4755fd:	4c 89 6d b0          	mov    %r13,-0x50(%rbp)
  475601:	4c 89 75 a8          	mov    %r14,-0x58(%rbp)
  475605:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  475609:	48 89 75 f0          	mov    %rsi,-0x10(%rbp)
  47560d:	48 89 55 e8          	mov    %rdx,-0x18(%rbp)
  475611:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  475615:	48 63 00             	movslq (%rax),%rax
  475618:	50                   	push   %rax
  475619:	48 c7 c0 03 00 00 00 	mov    $0x3,%rax
  475620:	49 89 c3             	mov    %rax,%r11
  475623:	58                   	pop    %rax
  475624:	4c 01 d8             	add    %r11,%rax
  475627:	48 98                	cltq
  475629:	50                   	push   %rax
  47562a:	48 c7 c0 fc ff ff ff 	mov    $0xfffffffffffffffc,%rax
  475631:	41 5b                	pop    %r11
  475633:	4c 21 d8             	and    %r11,%rax
  475636:	49 89 c3             	mov    %rax,%r11
  475639:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  47563d:	44 89 18             	mov    %r11d,(%rax)
  475640:	4c 89 d8             	mov    %r11,%rax
  475643:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  475647:	50                   	push   %rax
  475648:	48 63 00             	movslq (%rax),%rax
  47564b:	50                   	push   %rax
  47564c:	48 c7 c0 28 00 00 00 	mov    $0x28,%rax
  475653:	49 89 c3             	mov    %rax,%r11
  475656:	58                   	pop    %rax
  475657:	4c 01 d8             	add    %r11,%rax
  47565a:	49 89 c3             	mov    %rax,%r11
  47565d:	58                   	pop    %rax
  47565e:	44 89 18             	mov    %r11d,(%rax)
  475661:	4c 89 d8             	mov    %r11,%rax
  475664:	48 98                	cltq
  475666:	48 8d 05 1b 20 04 00 	lea    0x4201b(%rip),%rax        # 4b7688 <mainzone>
  47566d:	48 8b 00             	mov    (%rax),%rax
  475670:	48 83 c0 30          	add    $0x30,%rax
  475674:	48 8b 00             	mov    (%rax),%rax
  475677:	49 89 c4             	mov    %rax,%r12
  47567a:	4c 89 e0             	mov    %r12,%rax
  47567d:	48 83 c0 20          	add    $0x20,%rax
  475681:	48 8b 00             	mov    (%rax),%rax
  475684:	48 83 c0 08          	add    $0x8,%rax
  475688:	48 8b 00             	mov    (%rax),%rax
  47568b:	48 83 f8 00          	cmp    $0x0,%rax
  47568f:	0f 94 c0             	sete   %al
  475692:	0f b6 c0             	movzbl %al,%eax
  475695:	48 83 f8 00          	cmp    $0x0,%rax
  475699:	74 0d                	je     4756a8 <Z_Malloc+0xba>
  47569b:	4c 89 e0             	mov    %r12,%rax
  47569e:	48 83 c0 20          	add    $0x20,%rax
  4756a2:	48 8b 00             	mov    (%rax),%rax
  4756a5:	49 89 c4             	mov    %rax,%r12
  4756a8:	4c 89 e0             	mov    %r12,%rax
  4756ab:	49 89 c5             	mov    %rax,%r13
  4756ae:	4c 89 e0             	mov    %r12,%rax
  4756b1:	48 83 c0 20          	add    $0x20,%rax
  4756b5:	48 8b 00             	mov    (%rax),%rax
  4756b8:	49 89 c6             	mov    %rax,%r14
  4756bb:	4c 89 e8             	mov    %r13,%rax
  4756be:	50                   	push   %rax
  4756bf:	4c 89 f0             	mov    %r14,%rax
  4756c2:	49 89 c3             	mov    %rax,%r11
  4756c5:	58                   	pop    %rax
  4756c6:	4c 39 d8             	cmp    %r11,%rax
  4756c9:	0f 94 c0             	sete   %al
  4756cc:	0f b6 c0             	movzbl %al,%eax
  4756cf:	48 83 f8 00          	cmp    $0x0,%rax
  4756d3:	74 1d                	je     4756f2 <Z_Malloc+0x104>
  4756d5:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  4756d9:	48 63 00             	movslq (%rax),%rax
  4756dc:	50                   	push   %rax
  4756dd:	48 8d 05 c6 97 00 00 	lea    0x97c6(%rip),%rax        # 47eeaa <_IO_stdin_used+0x7eaa>
  4756e4:	48 89 c7             	mov    %rax,%rdi
  4756e7:	5e                   	pop    %rsi
  4756e8:	b8 00 00 00 00       	mov    $0x0,%eax
  4756ed:	e8 68 88 fa ff       	call   41df5a <I_Error>
  4756f2:	4c 89 e8             	mov    %r13,%rax
  4756f5:	48 83 c0 08          	add    $0x8,%rax
  4756f9:	48 8b 00             	mov    (%rax),%rax
  4756fc:	48 83 f8 00          	cmp    $0x0,%rax
  475700:	74 7f                	je     475781 <Z_Malloc+0x193>
  475702:	4c 89 e8             	mov    %r13,%rax
  475705:	48 83 c0 10          	add    $0x10,%rax
  475709:	48 63 00             	movslq (%rax),%rax
  47570c:	50                   	push   %rax
  47570d:	48 c7 c0 64 00 00 00 	mov    $0x64,%rax
  475714:	49 89 c3             	mov    %rax,%r11
  475717:	58                   	pop    %rax
  475718:	44 39 d8             	cmp    %r11d,%eax
  47571b:	0f 9c c0             	setl   %al
  47571e:	0f b6 c0             	movzbl %al,%eax
  475721:	48 83 f8 00          	cmp    $0x0,%rax
  475725:	74 12                	je     475739 <Z_Malloc+0x14b>
  475727:	4c 89 e8             	mov    %r13,%rax
  47572a:	48 83 c0 18          	add    $0x18,%rax
  47572e:	48 8b 00             	mov    (%rax),%rax
  475731:	49 89 c5             	mov    %rax,%r13
  475734:	49 89 c4             	mov    %rax,%r12
  475737:	eb 46                	jmp    47577f <Z_Malloc+0x191>
  475739:	4c 89 e0             	mov    %r12,%rax
  47573c:	48 83 c0 20          	add    $0x20,%rax
  475740:	48 8b 00             	mov    (%rax),%rax
  475743:	49 89 c4             	mov    %rax,%r12
  475746:	4c 89 e8             	mov    %r13,%rax
  475749:	50                   	push   %rax
  47574a:	48 c7 c0 28 00 00 00 	mov    $0x28,%rax
  475751:	49 89 c3             	mov    %rax,%r11
  475754:	58                   	pop    %rax
  475755:	4c 01 d8             	add    %r11,%rax
  475758:	48 89 c7             	mov    %rax,%rdi
  47575b:	b8 00 00 00 00       	mov    $0x0,%eax
  475760:	e8 01 fc ff ff       	call   475366 <Z_Free>
  475765:	4c 89 e0             	mov    %r12,%rax
  475768:	48 83 c0 18          	add    $0x18,%rax
  47576c:	48 8b 00             	mov    (%rax),%rax
  47576f:	49 89 c4             	mov    %rax,%r12
  475772:	4c 89 e0             	mov    %r12,%rax
  475775:	48 83 c0 18          	add    $0x18,%rax
  475779:	48 8b 00             	mov    (%rax),%rax
  47577c:	49 89 c5             	mov    %rax,%r13
  47577f:	eb 0d                	jmp    47578e <Z_Malloc+0x1a0>
  475781:	4c 89 e8             	mov    %r13,%rax
  475784:	48 83 c0 18          	add    $0x18,%rax
  475788:	48 8b 00             	mov    (%rax),%rax
  47578b:	49 89 c5             	mov    %rax,%r13
  47578e:	4c 89 e0             	mov    %r12,%rax
  475791:	48 83 c0 08          	add    $0x8,%rax
  475795:	48 8b 00             	mov    (%rax),%rax
  475798:	48 83 f8 00          	cmp    $0x0,%rax
  47579c:	75 2a                	jne    4757c8 <Z_Malloc+0x1da>
  47579e:	4c 89 e0             	mov    %r12,%rax
  4757a1:	48 63 00             	movslq (%rax),%rax
  4757a4:	50                   	push   %rax
  4757a5:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  4757a9:	48 63 00             	movslq (%rax),%rax
  4757ac:	49 89 c3             	mov    %rax,%r11
  4757af:	58                   	pop    %rax
  4757b0:	44 39 d8             	cmp    %r11d,%eax
  4757b3:	0f 9c c0             	setl   %al
  4757b6:	0f b6 c0             	movzbl %al,%eax
  4757b9:	48 83 f8 00          	cmp    $0x0,%rax
  4757bd:	75 09                	jne    4757c8 <Z_Malloc+0x1da>
  4757bf:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  4757c6:	eb 07                	jmp    4757cf <Z_Malloc+0x1e1>
  4757c8:	48 c7 c0 01 00 00 00 	mov    $0x1,%rax
  4757cf:	48 83 f8 00          	cmp    $0x0,%rax
  4757d3:	0f 85 e2 fe ff ff    	jne    4756bb <Z_Malloc+0xcd>
  4757d9:	4c 89 e0             	mov    %r12,%rax
  4757dc:	48 63 00             	movslq (%rax),%rax
  4757df:	50                   	push   %rax
  4757e0:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  4757e4:	48 63 00             	movslq (%rax),%rax
  4757e7:	49 89 c3             	mov    %rax,%r11
  4757ea:	58                   	pop    %rax
  4757eb:	4c 29 d8             	sub    %r11,%rax
  4757ee:	48 98                	cltq
  4757f0:	48 98                	cltq
  4757f2:	49 89 c5             	mov    %rax,%r13
  4757f5:	4c 89 e8             	mov    %r13,%rax
  4757f8:	48 98                	cltq
  4757fa:	50                   	push   %rax
  4757fb:	48 c7 c0 40 00 00 00 	mov    $0x40,%rax
  475802:	49 89 c3             	mov    %rax,%r11
  475805:	58                   	pop    %rax
  475806:	44 39 d8             	cmp    %r11d,%eax
  475809:	0f 9f c0             	setg   %al
  47580c:	0f b6 c0             	movzbl %al,%eax
  47580f:	48 83 f8 00          	cmp    $0x0,%rax
  475813:	0f 84 c3 00 00 00    	je     4758dc <Z_Malloc+0x2ee>
  475819:	4c 89 e0             	mov    %r12,%rax
  47581c:	50                   	push   %rax
  47581d:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  475821:	48 63 00             	movslq (%rax),%rax
  475824:	49 89 c3             	mov    %rax,%r11
  475827:	58                   	pop    %rax
  475828:	4c 01 d8             	add    %r11,%rax
  47582b:	49 89 c6             	mov    %rax,%r14
  47582e:	4c 89 e8             	mov    %r13,%rax
  475831:	48 98                	cltq
  475833:	50                   	push   %rax
  475834:	4c 89 f0             	mov    %r14,%rax
  475837:	41 5b                	pop    %r11
  475839:	44 89 18             	mov    %r11d,(%rax)
  47583c:	4c 89 d8             	mov    %r11,%rax
  47583f:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  475846:	48 98                	cltq
  475848:	50                   	push   %rax
  475849:	4c 89 f0             	mov    %r14,%rax
  47584c:	48 83 c0 08          	add    $0x8,%rax
  475850:	41 5b                	pop    %r11
  475852:	4c 89 18             	mov    %r11,(%rax)
  475855:	4c 89 d8             	mov    %r11,%rax
  475858:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  47585f:	50                   	push   %rax
  475860:	4c 89 f0             	mov    %r14,%rax
  475863:	48 83 c0 10          	add    $0x10,%rax
  475867:	41 5b                	pop    %r11
  475869:	44 89 18             	mov    %r11d,(%rax)
  47586c:	4c 89 d8             	mov    %r11,%rax
  47586f:	4c 89 e0             	mov    %r12,%rax
  475872:	50                   	push   %rax
  475873:	4c 89 f0             	mov    %r14,%rax
  475876:	48 83 c0 20          	add    $0x20,%rax
  47587a:	41 5b                	pop    %r11
  47587c:	4c 89 18             	mov    %r11,(%rax)
  47587f:	4c 89 d8             	mov    %r11,%rax
  475882:	4c 89 e0             	mov    %r12,%rax
  475885:	48 83 c0 18          	add    $0x18,%rax
  475889:	48 8b 00             	mov    (%rax),%rax
  47588c:	50                   	push   %rax
  47588d:	4c 89 f0             	mov    %r14,%rax
  475890:	48 83 c0 18          	add    $0x18,%rax
  475894:	41 5b                	pop    %r11
  475896:	4c 89 18             	mov    %r11,(%rax)
  475899:	4c 89 d8             	mov    %r11,%rax
  47589c:	4c 89 f0             	mov    %r14,%rax
  47589f:	50                   	push   %rax
  4758a0:	4c 89 f0             	mov    %r14,%rax
  4758a3:	48 83 c0 18          	add    $0x18,%rax
  4758a7:	48 8b 00             	mov    (%rax),%rax
  4758aa:	48 83 c0 20          	add    $0x20,%rax
  4758ae:	41 5b                	pop    %r11
  4758b0:	4c 89 18             	mov    %r11,(%rax)
  4758b3:	4c 89 d8             	mov    %r11,%rax
  4758b6:	4c 89 f0             	mov    %r14,%rax
  4758b9:	50                   	push   %rax
  4758ba:	4c 89 e0             	mov    %r12,%rax
  4758bd:	48 83 c0 18          	add    $0x18,%rax
  4758c1:	41 5b                	pop    %r11
  4758c3:	4c 89 18             	mov    %r11,(%rax)
  4758c6:	4c 89 d8             	mov    %r11,%rax
  4758c9:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  4758cd:	48 63 00             	movslq (%rax),%rax
  4758d0:	50                   	push   %rax
  4758d1:	4c 89 e0             	mov    %r12,%rax
  4758d4:	41 5b                	pop    %r11
  4758d6:	44 89 18             	mov    %r11d,(%rax)
  4758d9:	4c 89 d8             	mov    %r11,%rax
  4758dc:	48 8d 45 e8          	lea    -0x18(%rbp),%rax
  4758e0:	48 8b 00             	mov    (%rax),%rax
  4758e3:	48 83 f8 00          	cmp    $0x0,%rax
  4758e7:	74 3b                	je     475924 <Z_Malloc+0x336>
  4758e9:	48 8d 45 e8          	lea    -0x18(%rbp),%rax
  4758ed:	48 8b 00             	mov    (%rax),%rax
  4758f0:	50                   	push   %rax
  4758f1:	4c 89 e0             	mov    %r12,%rax
  4758f4:	48 83 c0 08          	add    $0x8,%rax
  4758f8:	41 5b                	pop    %r11
  4758fa:	4c 89 18             	mov    %r11,(%rax)
  4758fd:	4c 89 d8             	mov    %r11,%rax
  475900:	4c 89 e0             	mov    %r12,%rax
  475903:	50                   	push   %rax
  475904:	48 c7 c0 28 00 00 00 	mov    $0x28,%rax
  47590b:	49 89 c3             	mov    %rax,%r11
  47590e:	58                   	pop    %rax
  47590f:	4c 01 d8             	add    %r11,%rax
  475912:	50                   	push   %rax
  475913:	48 8d 45 e8          	lea    -0x18(%rbp),%rax
  475917:	48 8b 00             	mov    (%rax),%rax
  47591a:	41 5b                	pop    %r11
  47591c:	4c 89 18             	mov    %r11,(%rax)
  47591f:	4c 89 d8             	mov    %r11,%rax
  475922:	eb 4f                	jmp    475973 <Z_Malloc+0x385>
  475924:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
  475928:	48 63 00             	movslq (%rax),%rax
  47592b:	50                   	push   %rax
  47592c:	48 c7 c0 64 00 00 00 	mov    $0x64,%rax
  475933:	49 89 c3             	mov    %rax,%r11
  475936:	58                   	pop    %rax
  475937:	44 39 d8             	cmp    %r11d,%eax
  47593a:	0f 9d c0             	setge  %al
  47593d:	0f b6 c0             	movzbl %al,%eax
  475940:	48 83 f8 00          	cmp    $0x0,%rax
  475944:	74 14                	je     47595a <Z_Malloc+0x36c>
  475946:	48 8d 05 88 95 00 00 	lea    0x9588(%rip),%rax        # 47eed5 <_IO_stdin_used+0x7ed5>
  47594d:	48 89 c7             	mov    %rax,%rdi
  475950:	b8 00 00 00 00       	mov    $0x0,%eax
  475955:	e8 00 86 fa ff       	call   41df5a <I_Error>
  47595a:	48 c7 c0 02 00 00 00 	mov    $0x2,%rax
  475961:	48 98                	cltq
  475963:	50                   	push   %rax
  475964:	4c 89 e0             	mov    %r12,%rax
  475967:	48 83 c0 08          	add    $0x8,%rax
  47596b:	41 5b                	pop    %r11
  47596d:	4c 89 18             	mov    %r11,(%rax)
  475970:	4c 89 d8             	mov    %r11,%rax
  475973:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
  475977:	48 63 00             	movslq (%rax),%rax
  47597a:	50                   	push   %rax
  47597b:	4c 89 e0             	mov    %r12,%rax
  47597e:	48 83 c0 10          	add    $0x10,%rax
  475982:	41 5b                	pop    %r11
  475984:	44 89 18             	mov    %r11d,(%rax)
  475987:	4c 89 d8             	mov    %r11,%rax
  47598a:	4c 89 e0             	mov    %r12,%rax
  47598d:	48 83 c0 18          	add    $0x18,%rax
  475991:	48 8b 00             	mov    (%rax),%rax
  475994:	50                   	push   %rax
  475995:	48 8d 05 ec 1c 04 00 	lea    0x41cec(%rip),%rax        # 4b7688 <mainzone>
  47599c:	48 8b 00             	mov    (%rax),%rax
  47599f:	48 83 c0 30          	add    $0x30,%rax
  4759a3:	41 5b                	pop    %r11
  4759a5:	4c 89 18             	mov    %r11,(%rax)
  4759a8:	4c 89 d8             	mov    %r11,%rax
  4759ab:	48 c7 c0 11 4a 1d 00 	mov    $0x1d4a11,%rax
  4759b2:	50                   	push   %rax
  4759b3:	4c 89 e0             	mov    %r12,%rax
  4759b6:	48 83 c0 14          	add    $0x14,%rax
  4759ba:	41 5b                	pop    %r11
  4759bc:	44 89 18             	mov    %r11d,(%rax)
  4759bf:	4c 89 d8             	mov    %r11,%rax
  4759c2:	4c 89 e0             	mov    %r12,%rax
  4759c5:	50                   	push   %rax
  4759c6:	48 c7 c0 28 00 00 00 	mov    $0x28,%rax
  4759cd:	49 89 c3             	mov    %rax,%r11
  4759d0:	58                   	pop    %rax
  4759d1:	4c 01 d8             	add    %r11,%rax
  4759d4:	eb 00                	jmp    4759d6 <Z_Malloc+0x3e8>
  4759d6:	4c 8b 75 a8          	mov    -0x58(%rbp),%r14
  4759da:	4c 8b 6d b0          	mov    -0x50(%rbp),%r13
  4759de:	4c 8b 65 b8          	mov    -0x48(%rbp),%r12
  4759e2:	48 89 ec             	mov    %rbp,%rsp
  4759e5:	5d                   	pop    %rbp
  4759e6:	c3                   	ret

00000000004759e7 
