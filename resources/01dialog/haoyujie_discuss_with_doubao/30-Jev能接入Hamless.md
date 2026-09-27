
现在所谓的很火，目前它能够直接对接到现在主流的这些，所谓，Hamless 还是什么的，就是像 Cursor，像 Codex，还有 claude code 这样的前端的平台吗？
# 一句话总览
✅ **Claude Code、Codex 原生支持直接接入 Jev**；
✅ **Cursor 可以接入，但不是原生内置，需要手动搭 MCP 服务**；
> 你说的 Hamless 实际是 **Harness**，不是独立软件，是整套Agent运行框架（包含MCP、Skill、权限、循环控制器），Jev就是作为Harness里的**底层判断工具**嵌入进去。

## 1. Claude Code（Anthropic）
原生支持MCP（Model Context Protocol），官方直接提供TypeSafe的skill插件：
```bash
claude plugin marketplace add typesafe-ai/skills
claude plugin install typesafe@typesafe-ai
```
装好之后，Claude Code（写代码的Agent）在写代码、改文件、生成修复方案之后，**自动调用Jev**做校验：
- 判断AST等价性
- 判断断言真假
- 从多个修复方案里面选最优、给出概率
Claude作为系统2负责规划、写代码；Jev作为系统1做真假判定。

## 2. Codex（这里指Codex coding agent，不是老OpenAI Codex）
和Claude Code几乎一样，支持skills.sh，一行命令安装TypeSafe技能包，原生可以调用Jev API。

## 3. Cursor（VSCode插件）
Cursor**没有原生内置Jev**，但它支持MCP外部工具：
需要本地启动 `typesafe-jev-mcp` 这个MCP服务（npm包），然后在Cursor配置里添加这个MCP Server。
- 缺点：配置步骤多一点；
- 用途：Cursor生成代码、给出修复方案之后，把候选命题丢给Jev做真假判断。

## 重要关键点（一定要分清）
1. **Jev不会替代Cursor/Claude Code/Codex**
Jev不写代码，**只做封闭选项内的判定**。主Agent（Claude/Cursor）负责生成代码、构造假设、列出候选；Jev只负责快速校验、打分、二选一。

2. 接入本质：MCP协议
MCP = Model Context Protocol，是Anthropic推出的标准，让编码Agent可以调用外部工具。
TypeSafe官方发布了 `jev-mcp`，把Jev的三个原语（Choice / Score / Noul）封装成MCP工具，所有兼容MCP的coding agent都可以接入。

3. 限制
- Jev是云端API，**不能本地跑官方权重**，调用必须填TypeSafe的API Key；
- 所有判断任务，**必须由上层Agent预先构造候选选项**，Jev不能自己生成待验证命题；
- 它只能做单步原子判断，长链条证明必须靠外层Harness循环来回调度。

## 极简架构（Harness框架）
```
IDE前端(Cursor/Claude Code) → 主LLM Agent（系统2，写代码）
        ↓（MCP工具调用）
Jev（系统1，判定真假、选择、打分）
        ↓
Harness 控制器：收集Jev返回概率，决定下一步探索方向，维护符号上下文
```


如果你想，我们可以继续聊：MCP协议的底层交互逻辑，或者实际代码工程里，Jev做代码校验的失败案例。