再回头说 Transformer 和大模型。千问发布的时候我看到你有一些文件清单，这些清单中好像按你的意思来说有 Q、有 K、有 V，是吗？是三者都有的，还是说只有一种呢？
# 核心区分（最容易踩坑）
**磁盘上safetensors权重文件里存的是 Wq、Wk、Wv（投影矩阵，权重）；不是Q、K、V张量（推理时动态算出来的向量）。**
> Qwen2 / Qwen2.5 官方HF格式：**三层独立矩阵全部都有：q_proj.weight、k_proj.weight、v_proj.weight，再加 o_proj（输出投影）**。
每一层Transformer块，这4套矩阵都存在分片safetensors里面。

## 1. 名字对应
- `q_proj.weight`：Wq，把输入向量 → 乘这个矩阵，算出Q向量（Query）
- `k_proj.weight`：Wk，把输入向量 → 乘这个矩阵，算出K向量（Key）
- `v_proj.weight`：Wv，把输入向量 → 乘这个矩阵，算出V向量（Value）
- `o_proj.weight`：Wo，注意力加权求和之后，再过这个矩阵做输出

> 一句话：**权重文件保存“生成Q/K/V的计算公式（矩阵）”；当你输入prompt跑推理，实时矩阵乘法才算出Q、K、V张量（就是TransformerLens抓出来的那个）**。

## 2. Qwen是GQA（Grouped-Query Attention，分组查询注意力）
Qwen2.5-7B：28个Q头，但是**只有4组K/V头**。
- Wq矩阵尺寸更大（对应28个query头）
- Wk、Wv更小，只对应4个KV头；推理代码里再把KV复制扩展到和Q头数量一致
- 但是**权重文件里依然同时存在独立的 q_proj / k_proj / v_proj 三套矩阵，三者都保存在safetensors**，不是只有Q或者只有KV。

> 不要混淆：GQA只是**推理时K、V头数量更少**，不是磁盘上不存Wk/Wv。

## 3. 在索引文件里长这样（model.safetensors.index.json）
```json
"model.layers.0.self_attn.q_proj.weight": "model-00001-of-00004.safetensors",
"model.layers.0.self_attn.k_proj.weight": "model-00001-of-00004.safetensors",
"model.layers.0.self_attn.v_proj.weight": "model-00001-of-00004.safetensors",
"model.layers.0.self_attn.o_proj.weight": "model-00001-of-00004.safetensors",
```
每一层（layer 0，layer1 … layer27）都有这四个张量，分散在各个分片safetensors里。

## 4. 对比你前面关心的 TransformerLens
- 磁盘文件（safetensors）：**Wq/Wk/Wv（静态权重，永久不变）**
- 运行时（TransformerLens捕获）：**Q/K/V张量 = 输入向量 × Wq / Wk / Wv，每次prompt不同，结果张量就变**

举个极简公式：
$$
Q = XW_q,\quad K = XW_k,\quad V = XW_v
$$
X是该层输入向量（残差流）。

## 5. 补充：有些别的模型会把QKV合并成一个大矩阵（qkv_proj）
比如很多Llama的社区版本、部分推理引擎会把Wq,Wk,Wv拼接成单个大矩阵`qkv_proj.weight`。
👉 **但是Qwen官方原生HF包，是分开三个独立矩阵 q_proj / k_proj / v_proj，不是合并成一个qkv大张量**。这也是为什么大家做LoRA微调的时候，可以单独锁定/放开q_proj、k_proj、v_proj。

## 小结
1. ✅ 千问官方开源权重文件：**Wq、Wk、Wv三者全部存在，独立分开存储**；
2. ⚠️ 但是文件里**没有存Q、K、V张量**，张量是推理时实时计算产物；
3. Qwen是GQA，K/V头数量少于Q头，但权重矩阵Wk/Wv依然完整存在；
4. 这个分开的q/k/v投影布局，是HF生态标准，也正好适配LoRA单独对q_proj/k_proj/v_proj加低秩矩阵。

如果你想，下一步：
A：讲GQA的原理，为什么K/V头可以更少，显存怎么省；
B：或者回到TransformerLens，看怎么把Wq乘输入X，复现Q张量。