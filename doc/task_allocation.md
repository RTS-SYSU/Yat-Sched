# 考虑阻塞的任务分配机制介绍

随着实时系统中实现的功能变得越来越复杂，系统中的任务通常需要对同一对象进行操作，而这些对象必须以互斥的方式进行访问，包括代码段 、内存块、I/O 端口等。AUTOSAR 标准 @furst2009autosar 强制要求的自旋锁被广泛采用，用于保护共享资源上的计算免受竞争条件的影响，并应用资源共享协议来管理对共享资源的访问@brandenburg2022multiprocessor。 然而，在强制执行自旋锁的情况下，系统中的任务可能会由于共享资源的竞争而引发资源争用。 特别是当任务从不同的处理器请求相同的资源时，它们会在其主机处理器上忙等待，直到请求的资源被授予。 这会导致这些处理器中所有任务的潜在高阻塞，并可能显著破坏系统的可调度性@wieder2013spin。

为了减轻处理器之间的资源争用，我们提出了一种具有自旋锁管理共享资源的FPFPS (Fully-Partitioned Fixed-Priority Scheduling)多处理器系统的任务分配方法RAF，其中共享资源由来自其主机处理器的任务访问。RAF不依赖时间限制作为指导，而是利用资源竞争模型（RCM）来估计任务之间的资源争用程度。基于RCM的指导，RAF始终将资源争用最高的任务分配给同一处理器，以减少资源争用，从而减少任务的阻塞。

# 考虑阻塞的任务分配机制设计

1. *设计目标：*
- 减少资源争用
- 降低任务阻塞时间，提高系统可调度性
- 通用性和高效性

所提出的任务分配方法RAF利用了资源争用模型（RCM），该模型可以近似估计任务之间的资源争用程度，即争用系数（CF）。利用近似 CF 作为指导，该方法可根据以下基本原则做出分配决策：始终(i) 将 CF 近似值最高的任务分组；(ii) 将 CF 近似值最高的任务组分配给一个处理器。

2. *系统模型：*
设系统包含一组相同处理器 $Λ$ 和一组在 FP-FPS 方案下调度的随机任务 $Γ$ （sporadic task）。$Λ$ 中的第 $x$ 个处理器表示为 $λ_x$。 任务 $τ_i$（$Γ$ 中的第 $i$ 个任务）由 $τ_i = \{C_i, T_i, D_i, P_i, A_i\}$ 定义，其中 $C_i$ 是不访问共享资源的纯最坏情况执行时间，$T_i$ 是周期，$D_i$ 是 $D_i ≤ T_i$ 的约束期限，$P_i$ 是优先级，$A_i$ 是分配。 每个任务都有唯一的优先级，$P_i$ 值越高表示优先级越高。 函数 $Γ(λ_x)$ 返回分配给 $λ_x$ 的所有任务的集合。 为简单起见，我们使用 $U$ 来表示给定任务或任务组的利用率，例如分别为 #m-jaxon.render("U_{τ_i}", inline: true) 和 #m-jaxon.render("U_{Γ(λ_x)}", inline: true)。 函数 $|·|$ 表示给定集合的大小，例如 $|Λ|$ 给出系统中处理器的数量。 该系统还包含一组受自旋锁保护的共享资源 $R$ 。 对于资源 $r^k$（ $R$ 中的第 $k$ 个资源），$c^k$ 表示 $r^k$ 上的最坏情况计算时间，$N^k_i $ 给出由 $τ_i$ 在一次发布中向 $r^k$ 发出的请求的数量。 函数 $F(·)$ 表示给定任务所请求的资源。 多个处理器中的任务之间共享的资源称为全局资源，而在处理器内访问的资源称为本地资源。

3. *整体机制：*
对于一组给定的任务，RAF 会形成一组任务组 G，其中每个组都包含资源争用程度较高的任务。此外，还对所有任务组应用了一个利用率约束 #m-jaxon.render("\overline{U}", inline: true)，以约束一个组允许的最大利用率，即 #m-jaxon.render("\overline{U} \geq U_{\mathcal{G}_{a}},\forall\mathcal{G}_{a} \in G", inline: true) 。在这项工作中，#m-jaxon.render("\overline{U}", inline: true)被设置为系统在每个处理器中的平均利用率，即 #m-jaxon.render("\overline{U}", inline: true) = #m-jaxon.render("U_Γ/|Λ|", inline: true)。构建好 G 后，就可以根据任务组与处理器上现有任务之间的 CF 近似值进行分配，将任务组分配给处理器。

如上所述，RAF 中的组形成和分配决策都是基于 RCM 提供的 CF 近似做出的。 为了指导分配决策，RCM 需要为任意两个给定任务组生成 CF 近似值，表示为 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b)", inline: true)。 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b)", inline: true)值越高表示任务组之间的资源争用程度越高。 对于不共享任何资源的 #m-jaxon.render("\mathcal{G}_a", inline: true) 和 #m-jaxon.render("\mathcal{G}_b", inline: true) ，RCM 将通知分配器 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b) = 0", inline: true)。此外，具有单个任务 $τ_i$ 的输入被视为具有一个任务的一组，即 $\{τ_i\}$。

RAF 由基于资源争用的任务分组、处理器任务组的分配、资源争用模型（RCM）三个子部分组成：

*(1) 基于资源争用的任务分组*

任务分组技术遵循以下原则：始终合并具有最高 CF 近似值且强制执行 #m-jaxon.render("\overline{U}", inline: true) 的两个组。这样只有具有高资源争用的任务才会被合并到一组中，即使它们与系统中的其他任务共享某些资源。 此外，使用 #m-jaxon.render("\overline{U}", inline: true) 有效地避免了创建无法安装到任何处理器中的重型组。

#figure(
    image("images/算法1.png", width: 70%),
    caption: "Algorithm 1: Task group formation",
) <fig:tgf>

// #set text(style: "normal")
// #v(1em)#h(-2em) *Algorithm 1: Task group formation*
// ```c
// 1.  G = ∅;
// 2.  for each τ_i ∈ Γ do
// 3.    \mathcal{G}_a = {τ_i};  G = G ∪ \mathcal{G}_a;
// 4.  end
// 5.  compute Δ(\mathcal{G}_a, \mathcal{G}_b), ∀\mathcal{G}_a, \mathcal{G}_b ∈ G ∧ a ≠ b;
// 6.  while true do
// 7.    (\mathcal{G}_a, \mathcal{G}_b) = argmax{a, b} {Δ(\mathcal{G}_a, \mathcal{G}_b) | U_\mathcal{G}_a + U_\mathcal{G}_b ≤ U̅};
// 8.    if Δ(\mathcal{G}_a, \mathcal{G}_b) = 0 ∨ Δ(\mathcal{G}_a, \mathcal{G}_b) = ∅ then
// 9.       return G;
// 10.   end
// 11. \mathcal{G}_a = \mathcal{G}_a ∪ \mathcal{G}_b;  G = G \ \mathcal{G}_b;
// 12. Compute Δ(\mathcal{G}_a, \mathcal{G}_s), ∀\mathcal{G}_s ∈ G ∧ a ≠ s;
// 13. end
// 14. return G;
// ```
// #v(1em)
// #set text(style: "italic")
// #fake-par

算法 1（@fig:tgf） 给出了基于RCM的完整任务分组过程。 该算法以系统中的所有任务 $Γ$ 作为输入，初始化 $|Γ|$ 任务组，其中每个组包含一个任务（第 1-4 行）。 对于任何两个组 #m-jaxon.render("\mathcal{G}_a", inline: true) 和 #m-jaxon.render("\mathcal{G}_b", inline: true)， CF 近似值是使用 RCM 计算的，即第 5 行中的 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b)", inline: true)。然后，在满足 #m-jaxon.render("U_{\mathcal{G}_a}  + U_{\mathcal{G}_b} ≤ \overline{U}", inline: true) 的任意两个组中，找出 CF 近似值最高的一对组，开始组形成（第 7 行）。 如果这样的组对是通过正 CF 近似获得的，则通过合并两个组来创建一个新组（第 11 行）。 此外，还会计算新创建的组与 G 中现有组之间的 CF 近似值，以便在后续迭代中进一步形成组（第 12 行）。 这个过程一直持续到每个组独立于其他组（即 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b) = 0", inline: true)）或没有组可以在强制执行 #m-jaxon.render("\overline{U}", inline: true)的情况下合并为止（第 8-10 行）。 由于前一种情况，具有单一任务且不共享任何资源的组在整个过程中不会被合并。

特点 1：所提出的方法形成一组任务组，其中每个组中的任务具有最高的竞争性。 此外，#m-jaxon.render("\overline{U}", inline: true)还用于避免形成无法在单个处理器中分配的大型任务组。

*(2) 处理器任务组的分配*

#figure(
    image("images/算法2.png", width: 70%),
    caption: "Algorithm 2: Resource-aware task allocation",
) <fig:rta>

// #set text(style: "normal")
// #v(1em)#h(-2em) *Algorithm 2: Resource-aware task allocation*
// ```c
// 1. for each \mathcal{G}_a ∈ G do
// 2.   Ω_a = ∑{τ_i ∈ \mathcal{G}_a} Δ({τ_i}, \mathcal{G}_a \ τ_i);
// 3. end
// 4. for each x ∈ [1...min{|G|, |Λ|}] do
// 5.   \mathcal{G}_a = argmax{\mathcal{G}_a} {Ω_a | ∀\mathcal{G}_a ∈ G};
// 6.   A_i = λ_x, ∀τ_i ∈ \mathcal{G}_a;  G = G \ \mathcal{G}_a;
// 7. end
// 8. while G ≠ ∅ do
// 9.    λ_x = argmin{x} {U_Γ(λ_x) | ∀λ_x ∈ Λ};
// 10.   \mathcal{G}_a = argmax{\mathcal{G}_a} {Δ(\mathcal{G}_a, Γ(λ_x)) | ∀\mathcal{G}_a ∈ G};
// 11.   if U_λ_x + U_\mathcal{G}_a ≤ 1 then
// 12.    A_i = λ_x, ∀τ_i ∈ \mathcal{G}_a;  G = G \ \mathcal{G}_a;
// 13.   else
// 14.    for each τ_i ∈ \mathcal{G}_a, highest Δ({τ_i}, Γ(λ_x)) first do
// 15.     if U_λ_x + U_τ_i ≤ 1 then
// 16.      A_i = λ_x;  \mathcal{G}_a = \mathcal{G}_a \ τ_i;
// 17.     end
// 18.    end
// 19.    if no task in \mathcal{G}_a is allocated to λ_x then
// 20.     return unfeasible;
// 21.    end
// 22.   end
// 23. end
// 24. return {A_i, ∀τ_i ∈ Γ};
// ```#v(1em)
// #set text(style: "italic")
// #fake-par

构建 $G$ 后，执行算法 2（@fig:rta），将 $G$ 映射到一组处理器 $Λ$。该算法首先根据 CF 计算每个 #m-jaxon.render("\mathcal{G}_a", inline: true) 的权重 $Ω_a$（第 1-3 行）。这个权重反映了 #m-jaxon.render("\mathcal{G}_a", inline: true) 中任务之间内部资源竞争的程度。然后分两步进行分配。第一步基于 $Ω_a$ 对 $G$ 中最多 $|Λ|$ 组产生初始分配（第 4-7 行）。然后，第二步根据 RCM 的指导决定其余任务组的分配（第 8-23 行）。

在第一步中，该方法识别最多 $min\{|G|, |Λ|\}$ 个具有最高 $Ω_a$ 的任务组，并将这些组中的每一个分配给一个处理器（第 5-6 行）。对于具有相同 $Ω_a$ 的组，采用利用率较高的组。因为所有组都是在 #m-jaxon.render("\overline{U}", inline: true) 强制且 #m-jaxon.render("\overline{U}", inline: true) ≤ 1 的情况下形成的, 所以可行。通过在这些组中本地化任务，我们减轻了系统中具有最高 CF 近似值的任务之间的争用。

初始分配之后，第二步按照以下规则映射未分配的组：具有最小 #m-jaxon.render("U_{Γ(λ_x)}", inline: true) 的 $λ_x$ 优先，具有最高 #m-jaxon.render("Δ(\mathcal{G}_a, Г(λ_x))", inline: true) 的 #m-jaxon.render("\mathcal{G}_a", inline: true) 优先（第 9-10 行）。这增加了分配完整任务组的机会，并且可以减少一般情况下的资源争用。如果 #m-jaxon.render("\mathcal{G}_a", inline: true) 适合 $λ_x$，则组中的所有任务都分配给该处理器（第 11-12 行）。否则，为了提高分配方法的成功率并减少竞争，#m-jaxon.render("\mathcal{G}_a", inline: true) 中的任务被单独分配，其中具有最高 $Δ({τ_i}, Γ(λ_x))$ 的任务总是分配给 $λ_x$ 。然而，在这种情况下，如果 #m-jaxon.render("\mathcal{G}_a", inline: true) 中没有任务可以分配给 $λ_x$（即利用率最低的处理器），则算法立即返回，找不到可行的分配（第 19-21 行）。整个过程直到每项任务都得到分配，即 $A_i, ∀τ_i ∈ Γ$，才算结束。

特点 2：所提出的方法基于 RCM 将任务分配给处理器，而不需要特定的可调度性测试。

特点 3：所提出的方法下的任务不需要额外的运行时设施，例如 @yang2018resource 中的迁移，并且运行时开销可以忽略不计，相当于传统的 FPFPS 系统。

这总结了所提出的任务分配方法的描述。 该方法的时间复杂度为 $O(n^2)$，需要最多为$| |Г|×|Г| |$ 次迭代来 (i) 计算 G 中所有组对的 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b)", inline: true)，(ii) 形成任务组，以及 (iii) 将任务组分配给处理器。 正如分配过程所示，RCM 提供的 CF 近似值对于所提出的分配方法至关重要。


*(3) 资源争用模型（RCM）*

RCM 既没有为具有共享资源的任务提供安全的计时界限，也没有为任务之间的资源争用提供准确的量化。 RCM 的目标是考虑自旋锁的主要特征，在任务之间生成具有足够准确度的 CF 近似值，以便为任务分配提供有效的指导，从而减少争用。 同时，这样做也使得 RCM 可以快速地产生 CF 近似值，并与优先级分配算法、资源共享协议和可调度性测试分离。

如@wieder2013spin 中所述，根据资源请求的服务顺序，自旋锁可大致分为无序锁、先进先出锁和基于优先级的锁。 其中，FIFO 和基于优先级的自旋锁都强制规定特定的资源访问顺序，RCM 可以捕获该顺序以提供准确的 CF 近似值。 我们以资源访问顺序作为先验知识构建了细粒度的 RCM。

在 FIFO 自旋锁下，每个全局资源都与一个 FIFO 队列关联，该队列包含来自所有处理器的请求（基于其到达顺序），其中队列头部的任务始终被授予资源@zhao2018fifo 。这引出了引理 1。

引理 1：使用 FIFO 自旋锁，对 $r_k$ 的 $τ_i$ 请求最多可以被远程处理器对 $r_k$ 的请求阻塞一次。

证明：对于非抢占式自旋锁，资源访问严格遵循“鸽巢原理”，即处理器在任何时刻最多只能发出一个请求。 这在@wieder2013spin、@zhao2017new 中得到证明。 对于抢占式任务，较高优先级的任务可以抢占自旋任务（spinning task） $τ_i$ 以获得 $r_k$ ，但是这些任务在抢占 τi 后无法再请求 rk 。 否则，可能会发生死锁，其中 τi 无法被服务，因为它被 FIFO 队列中在 τi 之后等待（自旋）的任务抢占。 因此，处理器在任何时刻最多包含一个带有 FIFO 自旋锁的全局资源请求。

根据引理 1，在 FIFO 自旋锁下， $τ_i$  可能从 #m-jaxon.render("\mathcal{G}_a", inline: true) 中引起的争用在@eg:rcm1 中估计，表示为
#m-jaxon.render("φ^{fifo}(τ_i, \mathcal{G}_a)", inline: true) 。 符号 $N^k_i (t) $ 给出了在时间段 $t$ 内从 $τ_i$ 到 $r^k$ 的请求数量，即#m-jaxon.render("N^k_i(t) = \lceil \frac{t}{T_i}\\rceil × N^k_i", inline: true)。 假设 $τ_i$ 和 #m-jaxon.render("\mathcal{G}_a", inline: true) 分配在两个不同的处理器上，则 $τ_i$ 会因 #m-jaxon.render("\mathcal{G}_a", inline: true) 中的任务访问 $r^k$ 而导致最多  $N^k_i(T_i)$ 个阻塞。

#mitex(`
\phi^{fifo}(\tau_i, \mathcal{G}_a) = \sum_{r^k \in \mathbb{R}} \min \{ N_i^k(T_i), \sum_{\tau_j \in \mathcal{G}_a} N_j^k(T_i) \} \times c^k \quad
`) <eg:rcm1>

#fake-par

对于基于优先级的自旋锁，我们假设从 $τ_i$ 到 $r^k$ 的请求优先级是已知的，记为 $P^k_i$。 使用基于优先级的自旋锁，请求全局资源的任务按其访问优先级排序，并且始终向具有最高优先级的任务授予资源。 在这种情况下，$τ_i$ 可能会因所有对 $r^k$ 的高优先级请求和最多一个来自远程处理器的低优先级请求访问 $r^k$ 而受到阻塞。因此，假定 $τ_i$ 和 #m-jaxon.render("\mathcal{G}_a", inline: true) 被分配给两个不同的处理器，$τ_i$ 可能因 #m-jaxon.render("\mathcal{G}_a", inline: true) 产生的争用可通过@eg:rcm2 近似得出。 注意在@eg:rcm2 中添加了一个额外的请求来反映低优先级资源请求的潜在阻塞。

#mitex(`
\phi^{prio}(\tau_{i},\mathcal{G}_a)=\sum_{r^{k}\in F(\tau_{i})\cap F(\mathcal{G}_a)}\Big\{\Big(\sum_{\tau_{j} \in \mathcal{G}_a\wedge P_{i}^{k}<P_{j}^{k}}N_{j}^{k}(T_{i})\times c^{k}\Big)+c^{k}\Big\} \quad
`) <eg:rcm2>

#fake-par

通过@eg:rcm1 和@eg:rcm2，可以得到两种类型的自旋锁下 #m-jaxon.render("\mathcal{G}_a", inline: true) 和 #m-jaxon.render("\mathcal{G}_b", inline: true) 之间的 CF 近似值，如@eg:rcm3 所示。该公式计算了两组任务分配给不同处理器时可能产生的争用总和，数值越大，意味着将两组任务分配给同一处理器（如果可能的话）的收益越高。

#mitex(`
\Delta(\mathcal{G}_a,\mathcal{G}_b)=\sum_{\tau_i\in\mathcal{G}_a}\phi(\tau_i,\mathcal{G}_b)+\sum_{\tau_j\in\mathcal{G}_b}\phi(\tau_j,\mathcal{G}_a) \quad
`) <eg:rcm3>

#fake-par

RCM 中没有明确考虑本地高优先级任务的传递阻塞。 然而，通过引导分配，将具有较高 #m-jaxon.render("Δ(\mathcal{G}_a, \mathcal{G}_b)", inline: true) 的任务组映射到同一处理器，由于争用减少，它隐式地减轻了一般情况下的此类阻塞。 此外，这也避免了过于复杂的计算，因为计算可能需要有关底层系统的额外信息。

=== 考虑阻塞的任务分配机制实现
    RAF 为任务分配机制的核心函数，将任务分配给不同的分区。

    其主要步骤如下：
+ 初始化任务的分区和优先级。
+ 根据任务的截止时间对任务进行排序，并分配优先级。
+ 将访问资源的任务和不访问资源的任务分成两个列表。
+ 对访问资源的任务进行分组。
+ 将所有任务合理地放置在处理器上。
+ 先将分组（访问资源的任务）放在核心上。
+ 采用 Worst-Fit-Decreasing 将不访问资源的任务放在核心上

C语言实现RAF函数如下：
// `
//     - param tasksToAllocate 需要分配到不同分区的任务列表。
// 	  - param resources 任务可能需要的资源列表。
// 	  - param maxUtilPerCore 每个核心可以处理的最大利用率。
// 	  - return  任务对象列表的列表，其中每个列表代表一个分区，并包含分配给该分区的任务。
//    `

```c
  /**
	 * RAF方法将任务分配给不同的分区。
	 * 接收一个需要分配的任务列表，一个资源列表，总的分区数，以及每个核心的最大利用率作为输入。
	 * 返回一个SporadicTask对象的列表的列表，其中每个列表代表一个分区，并包含分配给该分区的任务。
	 *
	 * @param tasksToAllocate 需要分配到不同分区的任务列表。
	 * @param resources 任务可能需要的资源列表。
	 * @param total_partitions 可用于任务分配的总分区数（或核心数）。
	 * @param maxUtilPerCore 一个组允许的最大利用率。
	 * @return ArrayList<ArrayList<SporadicTask>> 这返回一个SporadicTask对象的列表的列表，其中每个列表代表一个分区，并包含分配给该分区的任务。
	 */
GPtrArray** RAF(GPtrArray* tasksToAllocate, GPtrArray* resources,
                             int total_partitions, double maxUtilPerCore)
{
    // 将所有属于tasksToAllocate的待分配任务的分区初始化置-1;
    for (int i = 0; i < tasksToAllocate->len; i++) {
        SporadicTask* sporadicTask = g_ptr_array_index(tasksToAllocate, i);
        sporadicTask->partition = -1; // 分区
        sporadicTask->priority = -1; // 优先级
        sporadicTask->Ri = 0; // 响应时间
    }

    // （初始化）建立最终返回分组
    GPtrArray** final_tasks = malloc(total_partitions * sizeof(GPtrArray*));
    for(int i = 0; i < total_partitions; i++) {
        final_tasks[i] = g_ptr_array_new();
    }

    // sort the tasks by the ddls 升序排序
    GPtrArray* tasks = g_ptr_array_sized_new(tasksToAllocate->len);
    g_ptr_array_add_range(tasks, tasksToAllocate->pdata, tasksToAllocate->len);
    g_ptr_array_sort(tasks, (GCompareFunc)compareSporadicTaskByDDL);

    // grant priority
    for(int i = 0; i < tasks->len; i++){
        SporadicTask* task = g_ptr_array_index(tasks, i);
        task->priority = tasks->len - i;
    }

    // 将访问资源的任务和不访问资源的任务分成两个list
    GPtrArray* tasksToAllocate_WithResource = g_ptr_array_new();
    GPtrArray* tasksToAllocate_NoResource = g_ptr_array_new();

    for(int i = 0; i < tasks->len; i++){
        SporadicTask* task = g_ptr_array_index(tasks, i);
        if(task->resource_required_index->len != 0){
            g_ptr_array_add(tasksToAllocate_WithResource, task);
        }
        else{
            g_ptr_array_add(tasksToAllocate_NoResource, task);
        }
    }

	/* Step1.对于访问资源的任务进行分组 */
	// 初始化每个task一个group
	GPtrArray* task_group_list = g_ptr_array_new_with_free_func((GDestroyNotify)g_ptr_array_unref);
    for(int i = 0; i < tasksToAllocate_WithResource->len; i++){
        SporadicTask* task = g_ptr_array_index(tasksToAllocate_WithResource, i);
        GPtrArray* task_group = g_ptr_array_new();
        g_ptr_array_add(task_group, task);
        g_ptr_array_add(task_group_list, task_group);
    }

	// 递归进行分组
    GPtrArray* group_list = Grouping(task_group_list, resources, maxUtilPerCore);

	// CF 大到小
	g_ptr_array_sort_with_data(group_list, (GCompareDataFunc)compareCF, resources);

	/* Step2.将所有任务合理地放置在processor上 */
	// 每个核心的利用率初始化为0
	double* utilPerPartition = (double*)malloc(total_partitions * sizeof(double));
	for (int i = 0; i < total_partitions; i++) {
		utilPerPartition[i] = 0.0;
	}

    /* Step2-1.先将分组(访问资源的任务)放在核心上 */
    // 分组数小于等于核心数
    if(group_list->len <= total_partitions){
        for(int i = 0 ; i < group_list->len; i++ ){
            // 第i个group放在第i个核心上,并更新每个核心的利用率
            GPtrArray* group = g_ptr_array_index(group_list, i);
            for(int j = 0; j < group->len; j++){
                SporadicTask* task_in_group = g_ptr_array_index(group, j);
                task_in_group->partition = i;
                g_ptr_array_add(final_tasks[i], task_in_group);
                utilPerPartition[i] += task_in_group->util;
            }
        }
    }
    // 分组数大于核心数
    else {
        for(int i = 0 ; i < total_partitions; i++ ){
            GPtrArray* group = g_ptr_array_index(group_list, i);
            for(int j = 0; j < group->len; j++){
                SporadicTask* task_in_group = g_ptr_array_index(group, j);
                // 第i个group放在第i个核心上（前total_partition）
                task_in_group->partition = i;
                g_ptr_array_add(final_tasks[i], task_in_group);
                utilPerPartition[i] += task_in_group->util;
            }
        }
        // 分好的全移除
        for(int i = 0 ; i < total_partitions; i++){
            g_ptr_array_remove_index(group_list, 0);
        }
        // 递归分配
        if(!RAFAllocating(group_list, resources, utilPerPartition, total_partitions, final_tasks)){
            return NULL;
        }
    }

    /* Step2-2.采用Worst-Fit-Decreasing将不访问资源的任务放在核心上 */
    // 按利用率从大到小排序（降序）
    g_ptr_array_sort(tasksToAllocate_NoResource, (GCompareFunc)compareSporadicTaskByUtil);

    for(int i = 0; i < tasksToAllocate_NoResource->len; i++) {
        SporadicTask* task = g_ptr_array_index(tasksToAllocate_NoResource, i);
        int target = -1;
        double minUtil = 2.0;

        // 找到利用率最小的核心作为本次分配的目标
        for(int j = 0; j < total_partitions; j++) {
            if(minUtil > utilPerPartition[j]) {
                minUtil = utilPerPartition[j];
                target = j;
            }
        }

        if(target == -1) {
            fprintf(stderr, "RAF error!\n");
            return NULL;
        }

        // 当前任务的利用率（task->util）小于或等于目标处理器的剩余利用率（1 - minUtil）则分配
        if((double)1 - minUtil >= task->util) {
            task->partition = target;
            g_ptr_array_add(final_tasks[target], task);
            utilPerPartition[target] += task->util;
        } else {
            fprintf(stderr, "RAF error!\n");
            return NULL;
        }
    }

    // 在不再需要 GPtrArray 时释放内存
    g_ptr_array_unref(tasks);
    g_ptr_array_unref(tasksToAllocate_WithResource);
    g_ptr_array_unref(tasksToAllocate_NoResource);
    g_ptr_array_unref(task_group_list);
    g_ptr_array_unref(group_list);

    // 返回最终的任务分组
    return final_tasks;
}

```

Grouping 函数根据任务的竞争因子（CF）对任务进行分组。该函数是算法 1（@fig:tgf）的具体实现，将具有最高 CF 的组合并为一个，只要新组的总利用率不超过每个核心的最大利用率。

`
     - param task_group_list 一个 SporadicTask 对象的列表的列表。每个列表代表一组任务。
     - param resources 一个 Resource 对象的列表。这些是任务可能需要访问的资源。
     - param maxUtilPerCore 每个处理器核心允许的最大利用率。
     - return 分组后的任务对象列表的列表。每个列表代表一组任务。`


```c
/**
 * @param task_group_list 访问资源的任务对象列表的列表。每个列表代表一组任务。
 * @param resources 任务可能需要的资源列表。
 * @param maxUtilPerCore 每个处理器核心允许的最大利用率。
 *
 * @return 返回一个新的任务组列表，其中每个任务组的利用率不超过maxUtilPerCore。
 */
GPtrArray* Grouping(GPtrArray* task_group_list, GPtrArray* resources, double maxUtilPerCore);`
```
Grouping 函数的主要步骤如下：
+ 计算 task_group_list 中每对组的 CF 并记录下来。
+ 按照 CF 的降序对任务组对进行排序。
+ 试图合并 CF 最高的一对任务组。如果合并后的利用率不超过maxUtilPerCore，则进行合并。
+ 合并后，从 task_group_list 中删除原始的两个组并添加新的组。
+ 重复该过程，直到不能再合并更多的组。
+ 算法结束，返回当前的任务组列表。

`
`
RAFAllocating函数用于将任务组分配到各个处理器上。

// `
//      - param group_list 任务对象列表的列表。每个列表代表一组任务。
//      - param resources 任务可能需要的资源列表。
//      - param utilPerPartition Double对象的列表，表示每个分区的利用率。
//      - param total_partitions 整数，表示总的分区数。
//      - param final_tasks 任务对象列表的列表，表示最终分配的任务。
//      - return 布尔值，如果所有任务组都已成功分配，则返回 true，否则返回 false。`

```c
/**
 * @param group_list 待分配的访问资源的任务组列表。
 * @param resources 任务可能需要的资源列表。
 * @param utilPerPartition 每个处理器核心（分区）的利用率列表。
 * @param total_partitions 处理器核心的总数。
 * @param final_tasks 任务对象列表的列表，表示最终的任务分配结果。
 *
 * @return 如果所有任务组都成功分配到处理器核心上，返回正数；否则，返回负数。
 */
int RAFAllocating(GPtrArray* group_list, GPtrArray* resources, double* utilPerPartition, int total_partitions, GPtrArray* final_tasks);
```

RAFAllocating 函数的主要步骤如下：
+ 如果所有任务组都已分配，返回 true。
+ 找出剩余利用率最高的核心。
+ 根据该核心上的任务和 group_list 中所有组的 CF 进行排序。
+ 如果 CF 最大的组可以放入剩余利用率最高的核心，则将该组分配到该核心，并更新利用率数组。
+ 如果不能将整个组放入核心，则将任务组内的任务按contention factor从大到小排序，并尝试将组内的单个任务分配到核心，直到不能再放入更多任务。
+ 如果一个任务都不能分配，则返回 false。
+ 递归调用自身，继续分配剩余的任务组。

