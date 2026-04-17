<Z_Free>:
  475366:	55                   	push   %rbp
  475367:	48 89 e5             	mov    %rsp,%rbp
  47536a:	48 81 ec 00 01 00 00 	sub    $0x100,%rsp
  475371:	4c 89 65 e0          	mov    %r12,-0x20(%rbp)
  475375:	4c 89 6d d8          	mov    %r13,-0x28(%rbp)
  475379:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  47537d:	48 8d 45 f8          	lea    -0x8(%rbp),%rax
  475381:	48 8b 00             	mov    (%rax),%rax
  475384:	50                   	push   %rax
  475385:	48 c7 c0 28 00 00 00 	mov    $0x28,%rax
  47538c:	49 89 c3             	mov    %rax,%r11
  47538f:	58                   	pop    %rax
  475390:	4c 29 d8             	sub    %r11,%rax
  475393:	49 89 c4             	mov    %rax,%r12
  475396:	4c 89 e0             	mov    %r12,%rax
  475399:	48 83 c0 14          	add    $0x14,%rax
  47539d:	48 63 00             	movslq (%rax),%rax
  4753a0:	50                   	push   %rax
  4753a1:	48 c7 c0 11 4a 1d 00 	mov    $0x1d4a11,%rax
  4753a8:	49 89 c3             	mov    %rax,%r11
  4753ab:	58                   	pop    %rax
  4753ac:	44 39 d8             	cmp    %r11d,%eax
  4753af:	0f 95 c0             	setne  %al
  4753b2:	0f b6 c0             	movzbl %al,%eax
  4753b5:	48 83 f8 00          	cmp    $0x0,%rax
  4753b9:	74 14                	je     4753cf <Z_Free+0x69>
  4753bb:	48 8d 05 c1 9a 00 00 	lea    0x9ac1(%rip),%rax        # 47ee83 <_IO_stdin_used+0x7e83>
  4753c2:	48 89 c7             	mov    %rax,%rdi
  4753c5:	b8 00 00 00 00       	mov    $0x0,%eax
  4753ca:	e8 8b 8b fa ff       	call   41df5a <I_Error>
  4753cf:	4c 89 e0             	mov    %r12,%rax
  4753d2:	48 83 c0 08          	add    $0x8,%rax
  4753d6:	48 8b 00             	mov    (%rax),%rax
  4753d9:	50                   	push   %rax
  4753da:	48 c7 c0 00 01 00 00 	mov    $0x100,%rax
  4753e1:	48 98                	cltq
  4753e3:	49 89 c3             	mov    %rax,%r11
  4753e6:	58                   	pop    %rax
  4753e7:	4c 39 d8             	cmp    %r11,%rax
  4753ea:	0f 9f c0             	setg   %al
  4753ed:	0f b6 c0             	movzbl %al,%eax
  4753f0:	48 83 f8 00          	cmp    $0x0,%rax
  4753f4:	74 1c                	je     475412 <Z_Free+0xac>
  4753f6:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  4753fd:	48 98                	cltq
  4753ff:	50                   	push   %rax
  475400:	4c 89 e0             	mov    %r12,%rax
  475403:	48 83 c0 08          	add    $0x8,%rax
  475407:	48 8b 00             	mov    (%rax),%rax
  47540a:	41 5b                	pop    %r11
  47540c:	4c 89 18             	mov    %r11,(%rax)
  47540f:	4c 89 d8             	mov    %r11,%rax
  475412:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  475419:	48 98                	cltq
  47541b:	50                   	push   %rax
  47541c:	4c 89 e0             	mov    %r12,%rax
  47541f:	48 83 c0 08          	add    $0x8,%rax
  475423:	41 5b                	pop    %r11
  475425:	4c 89 18             	mov    %r11,(%rax)
  475428:	4c 89 d8             	mov    %r11,%rax
  47542b:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  475432:	50                   	push   %rax
  475433:	4c 89 e0             	mov    %r12,%rax
  475436:	48 83 c0 10          	add    $0x10,%rax
  47543a:	41 5b                	pop    %r11
  47543c:	44 89 18             	mov    %r11d,(%rax)
  47543f:	4c 89 d8             	mov    %r11,%rax
  475442:	48 c7 c0 00 00 00 00 	mov    $0x0,%rax
  475449:	50                   	push   %rax
  47544a:	4c 89 e0             	mov    %r12,%rax
  47544d:	48 83 c0 14          	add    $0x14,%rax
  475451:	41 5b                	pop    %r11
  475453:	44 89 18             	mov    %r11d,(%rax)
  475456:	4c 89 d8             	mov    %r11,%rax
  475459:	4c 89 e0             	mov    %r12,%rax
  47545c:	48 83 c0 20          	add    $0x20,%rax
  475460:	48 8b 00             	mov    (%rax),%rax
  475463:	49 89 c5             	mov    %rax,%r13
  475466:	4c 89 e8             	mov    %r13,%rax
  475469:	48 83 c0 08          	add    $0x8,%rax
  47546d:	48 8b 00             	mov    (%rax),%rax
  475470:	48 83 f8 00          	cmp    $0x0,%rax
  475474:	0f 94 c0             	sete   %al
  475477:	0f b6 c0             	movzbl %al,%eax
  47547a:	48 83 f8 00          	cmp    $0x0,%rax
  47547e:	0f 84 9c 00 00 00    	je     475520 <Z_Free+0x1ba>
  475484:	4c 89 e8             	mov    %r13,%rax
  475487:	50                   	push   %rax
  475488:	8b 00                	mov    (%rax),%eax
  47548a:	50                   	push   %rax
  47548b:	4c 89 e0             	mov    %r12,%rax
  47548e:	48 63 00             	movslq (%rax),%rax
  475491:	49 89 c3             	mov    %rax,%r11
  475494:	58                   	pop    %rax
  475495:	4c 01 d8             	add    %r11,%rax
  475498:	49 89 c3             	mov    %rax,%r11
  47549b:	58                   	pop    %rax
  47549c:	44 89 18             	mov    %r11d,(%rax)
  47549f:	4c 89 d8             	mov    %r11,%rax
  4754a2:	48 98                	cltq
  4754a4:	4c 89 e0             	mov    %r12,%rax
  4754a7:	48 83 c0 18          	add    $0x18,%rax
  4754ab:	48 8b 00             	mov    (%rax),%rax
  4754ae:	50                   	push   %rax
  4754af:	4c 89 e8             	mov    %r13,%rax
  4754b2:	48 83 c0 18          	add    $0x18,%rax
  4754b6:	41 5b                	pop    %r11
  4754b8:	4c 89 18             	mov    %r11,(%rax)
  4754bb:	4c 89 d8             	mov    %r11,%rax
  4754be:	4c 89 e8             	mov    %r13,%rax
  4754c1:	50                   	push   %rax
  4754c2:	4c 89 e8             	mov    %r13,%rax
  4754c5:	48 83 c0 18          	add    $0x18,%rax
  4754c9:	48 8b 00             	mov    (%rax),%rax
  4754cc:	48 83 c0 20          	add    $0x20,%rax
  4754d0:	41 5b                	pop    %r11
  4754d2:	4c 89 18             	mov    %r11,(%rax)
  4754d5:	4c 89 d8             	mov    %r11,%rax
  4754d8:	4c 89 e0             	mov    %r12,%rax
  4754db:	50                   	push   %rax
  4754dc:	48 8d 05 a5 21 04 00 	lea    0x421a5(%rip),%rax        # 4b7688 <mainzone>
  4754e3:	48 8b 00             	mov    (%rax),%rax
  4754e6:	48 83 c0 30          	add    $0x30,%rax
  4754ea:	48 8b 00             	mov    (%rax),%rax
  4754ed:	49 89 c3             	mov    %rax,%r11
  4754f0:	58                   	pop    %rax
  4754f1:	4c 39 d8             	cmp    %r11,%rax
  4754f4:	0f 94 c0             	sete   %al
  4754f7:	0f b6 c0             	movzbl %al,%eax
  4754fa:	48 83 f8 00          	cmp    $0x0,%rax
  4754fe:	74 1a                	je     47551a <Z_Free+0x1b4>
  475500:	4c 89 e8             	mov    %r13,%rax
  475503:	50                   	push   %rax
  475504:	48 8d 05 7d 21 04 00 	lea    0x4217d(%rip),%rax        # 4b7688 <mainzone>
  47550b:	48 8b 00             	mov    (%rax),%rax
  47550e:	48 83 c0 30          	add    $0x30,%rax
  475512:	41 5b                	pop    %r11
  475514:	4c 89 18             	mov    %r11,(%rax)
  475517:	4c 89 d8             	mov    %r11,%rax
  47551a:	4c 89 e8             	mov    %r13,%rax
  47551d:	49 89 c4             	mov    %rax,%r12
  475520:	4c 89 e0             	mov    %r12,%rax
  475523:	48 83 c0 18          	add    $0x18,%rax
  475527:	48 8b 00             	mov    (%rax),%rax
  47552a:	49 89 c5             	mov    %rax,%r13
  47552d:	4c 89 e8             	mov    %r13,%rax
  475530:	48 83 c0 08          	add    $0x8,%rax
  475534:	48 8b 00             	mov    (%rax),%rax
  475537:	48 83 f8 00          	cmp    $0x0,%rax
  47553b:	0f 94 c0             	sete   %al
  47553e:	0f b6 c0             	movzbl %al,%eax
  475541:	48 83 f8 00          	cmp    $0x0,%rax
  475545:	0f 84 96 00 00 00    	je     4755e1 <Z_Free+0x27b>
  47554b:	4c 89 e0             	mov    %r12,%rax
  47554e:	50                   	push   %rax
  47554f:	8b 00                	mov    (%rax),%eax
  475551:	50                   	push   %rax
  475552:	4c 89 e8             	mov    %r13,%rax
  475555:	48 63 00             	movslq (%rax),%rax
  475558:	49 89 c3             	mov    %rax,%r11
  47555b:	58                   	pop    %rax
  47555c:	4c 01 d8             	add    %r11,%rax
  47555f:	49 89 c3             	mov    %rax,%r11
  475562:	58                   	pop    %rax
  475563:	44 89 18             	mov    %r11d,(%rax)
  475566:	4c 89 d8             	mov    %r11,%rax
  475569:	48 98                	cltq
  47556b:	4c 89 e8             	mov    %r13,%rax
  47556e:	48 83 c0 18          	add    $0x18,%rax
  475572:	48 8b 00             	mov    (%rax),%rax
  475575:	50                   	push   %rax
  475576:	4c 89 e0             	mov    %r12,%rax
  475579:	48 83 c0 18          	add    $0x18,%rax
  47557d:	41 5b                	pop    %r11
  47557f:	4c 89 18             	mov    %r11,(%rax)
  475582:	4c 89 d8             	mov    %r11,%rax
  475585:	4c 89 e0             	mov    %r12,%rax
  475588:	50                   	push   %rax
  475589:	4c 89 e0             	mov    %r12,%rax
  47558c:	48 83 c0 18          	add    $0x18,%rax
  475590:	48 8b 00             	mov    (%rax),%rax
  475593:	48 83 c0 20          	add    $0x20,%rax
  475597:	41 5b                	pop    %r11
  475599:	4c 89 18             	mov    %r11,(%rax)
  47559c:	4c 89 d8             	mov    %r11,%rax
  47559f:	4c 89 e8             	mov    %r13,%rax
  4755a2:	50                   	push   %rax
  4755a3:	48 8d 05 de 20 04 00 	lea    0x420de(%rip),%rax        # 4b7688 <mainzone>
  4755aa:	48 8b 00             	mov    (%rax),%rax
  4755ad:	48 83 c0 30          	add    $0x30,%rax
  4755b1:	48 8b 00             	mov    (%rax),%rax
  4755b4:	49 89 c3             	mov    %rax,%r11
  4755b7:	58                   	pop    %rax
  4755b8:	4c 39 d8             	cmp    %r11,%rax
  4755bb:	0f 94 c0             	sete   %al
  4755be:	0f b6 c0             	movzbl %al,%eax
  4755c1:	48 83 f8 00          	cmp    $0x0,%rax
  4755c5:	74 1a                	je     4755e1 <Z_Free+0x27b>
  4755c7:	4c 89 e0             	mov    %r12,%rax
  4755ca:	50                   	push   %rax
  4755cb:	48 8d 05 b6 20 04 00 	lea    0x420b6(%rip),%rax        # 4b7688 <mainzone>
  4755d2:	48 8b 00             	mov    (%rax),%rax
  4755d5:	48 83 c0 30          	add    $0x30,%rax
  4755d9:	41 5b                	pop    %r11
  4755db:	4c 89 18             	mov    %r11,(%rax)
  4755de:	4c 89 d8             	mov    %r11,%rax
  4755e1:	4c 8b 6d d8          	mov    -0x28(%rbp),%r13
  4755e5:	4c 8b 65 e0          	mov    -0x20(%rbp),%r12
  4755e9:	48 89 ec             	mov    %rbp,%rsp
  4755ec:	5d                   	pop    %rbp
  4755ed:	c3                   	ret

00000000004755ee 
