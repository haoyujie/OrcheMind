// OMStudio C# P/Invoke 互操作示例
// 用法：把本文件加入现有 C# 编辑器工程，加载 omstudio.dll（与编辑器同目录），
// 即可在 2D 编辑器内获得 3D 拓扑投影 + 拾取能力，无需自行实现降维与空间索引。
//
// 集成建议（对应三个难点的桥接）：
//  1) 本体数据：C# 侧仍是唯一数据源，C++ 侧只持渲染必需字段；
//     新增/修改/删除时调用 AddEntity/UpdateEntity/RemoveEntity 增量同步。
//  2) 拾取：把鼠标屏幕坐标换算成 NDC 后调 PickScreen（内部走八叉树，O(logN) 级）。
//  3) 聚焦：Select(id) 后 C++ 自动把该对象设为拓扑中心并重投影。
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace OrcheMind.OMStudio
{
    public static class Native
    {
        private const string Dll = "omstudio.dll";

        [StructLayout(LayoutKind.Sequential)]
        public struct OmEntity
        {
            public ulong Id;          // 0 = 由引擎分配
            public IntPtr Dims;       // float*，5~6 维语素向量
            public int DimCount;
            public float Radius;
            public uint Color;        // 0xRRGGBB
            public ulong ParentId;
            [MarshalAs(UnmanagedType.LPStr)] public string Label;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct RenderNode
        {
            public ulong Id;
            public float X, Y, Z;
            public float Radius;
            public uint Color;
            public int Lod;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct RenderEdge
        {
            public float AX, AY, AZ, BX, BY, BZ;
            public float Strength;
        }

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_init([MarshalAs(UnmanagedType.LPStr)] string topology, int dim);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_shutdown();
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_set_topology([MarshalAs(UnmanagedType.LPStr)] string topology);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr om_topology_name();
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_reproject();

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong om_add_entity(ref OmEntity e);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern int om_remove_entity(ulong id);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern int om_get_entity(ulong id, ref OmEntity e);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_update_entity(ulong id, float[] dims, int dimCount,
                                                   float radius, uint color);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong om_entity_count();
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong om_selected();
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_select(ulong id);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_clear_selection();

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_set_relation(ulong a, ulong b, float strength);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void om_set_relation_verb(ulong a, ulong b, float strength, string verb);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern int om_remove_relation(ulong a, ulong b);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_set_camera(float eyeX, float eyeY, float eyeZ,
                                                float tgtX, float tgtY, float tgtZ,
                                                float upX, float upY, float upZ,
                                                float fovYDeg, float aspect,
                                                float zn, float zf);
        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong om_pick_screen(float ndcX, float ndcY);

        [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void om_collect_visible(float maxDist, float edgeStrengthThreshold,
                                                     [In, Out] RenderNode[] nodes, ref int nodeCount,
                                                     [In, Out] RenderEdge[] edges, ref int edgeCount);
    }

    /// <summary>在现有 C# 2D 编辑器里使用 3D 拓扑视图的封装示例</summary>
    public static class OmStudioBridge
    {
        // 属性面板数据源仍在 C#（本体存储不迁移）
        public static void Init(string topology, int dim) => Native.om_init(topology, dim);

        public static ulong AddEntity(float[] dims, float radius, uint color, ulong parentId, string label)
        {
            var dimsPtr = Marshal.AllocHGlobal(dims.Length * sizeof(float));
            try
            {
                Marshal.Copy(dims, 0, dimsPtr, dims.Length);
                var e = new Native.OmEntity
                {
                    Id = 0,
                    Dims = dimsPtr,
                    DimCount = dims.Length,
                    Radius = radius,
                    Color = color,
                    ParentId = parentId,
                    Label = label
                };
                return Native.om_add_entity(ref e);
            }
            finally
            {
                Marshal.FreeHGlobal(dimsPtr);
            }
        }

        public static void RemoveEntity(ulong id) => Native.om_remove_entity(id);

        /// <summary>鼠标屏幕坐标 → 拾取实体（onClick 时调用；ndc 由编辑器换算）</summary>
        public static ulong Pick(float ndcX, float ndcY) => Native.om_pick_screen(ndcX, ndcY);

        /// <summary>选中并聚焦（C++ 侧自动执行拓扑中心重投影）</summary>
        public static void SelectAndFocus(ulong id) => Native.om_select(id);

        /// <summary>在选中对象下"生长"一个新对象：复制父向量 + 噪声，再入场景</summary>
        public static ulong GrowChild(ulong parentId, ulong engineId, Random rng)
        {
            var dims = new float[6];
            if (engineId != 0) { /* 可先读父实体向量：Native.om_get_entity(...) */ }
            for (int i = 0; i < dims.Length; i++)
                dims[i] = (float)(rng.NextDouble() * 2 - 1);
            dims[4] += 0.2f; // 径向偏置：向外生长
            var id = AddEntity(dims, 0.06f, 0xFFD25A, parentId, "morpheme_grown");
            Native.om_set_relation(parentId, id, 0.85f);
            Native.om_select(id); // 新对象自动聚焦
            return id;
        }

        /// <summary>切换构型（编辑器菜单项：环面 / 双曲）</summary>
        public static void SwitchTopology(string name) => Native.om_set_topology(name);
    }
}
