/*!
* Copyright 2007  Technical University of Catalonia
*
* Use, modification and distribution is subject to the Boost Software
* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*
*  Authors: Dmitry Bufistov
*           Andrey Parfenov
*/

#ifndef BOOST_GRAPH_HOWARD_CYCLE_RATIO_HOWARD_HPP
#define BOOST_GRAPH_HOWARD_CYCLE_RATIO_HOWARD_HPP

/*!
* \file Maximum cycle ratio algorithm (Jean Cochet-Terrasson, Guy
* Cochen and others) 
*/
#include <exception>
#include <set> 
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/concept_check.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/iteration_macros.hpp>

namespace boost {
  namespace detail {
    /// To avoid round error.
    static const double mcr_howard_ltolerance = 0.00001; 

    /*!
     * Calculate maximum cycle ratio of "good" directed multigraph
     * g. Use Howard's iteration policy algorithm ("Numerical
     * Computation of Spectral Elements in MAX-PLUS algebra" by Jean
     * Cochet-Terrasson, Guy Cochen and others).
     *
     * \param g = (V, E) - a "good" directed multigraph (out_degree of
     * each vertex is greater then 0). If graph is strongly connected
     * then it is "good".
     *
     * \param vim - Vertex Index, read property Map: V -> [0,
     * num_vertices(g)).
     *
     * \param ewm - edge weight read property map: E -> R
     *
     * \param ewm2 - edge weight2 read property map: E -> R+
     *
     * \return maximum_{for all cycles C}CR(C), or
     * -(std::numeric_limits<double>)::max() if g is not "good".
     */
    template <typename TGraph, typename TVertexIndexMap, 
              typename TWeight1EdgeMap, typename TWeight2EdgeMap >      
    class Cmcr_Howard 
    {
    public:
      Cmcr_Howard(const TGraph& g, TVertexIndexMap vim, TWeight1EdgeMap ewm, 
                  TWeight2EdgeMap ew2m) 
        : m_g(g), m_vim(vim), m_ew1m(ewm), m_ew2m(ew2m),
          m_g2pi_g_vm(std::vector<pi_vertex_t>().end(), m_vim), /// Stupid dummy initialization
          m_minus_infinity(-(std::numeric_limits<double>::max)())
      {
        typedef typename boost::graph_traits<TGraph>::directed_category DirCat;
        BOOST_STATIC_ASSERT((boost::is_convertible<DirCat*, boost::directed_tag*>::value == true));
        m_cr = m_minus_infinity;
      }
        
      double operator()() 
      {
        return maximum_cycle_ratio_Howard(); 
      }

      virtual ~Cmcr_Howard() { }

    protected:
      typedef typename boost::graph_traits<TGraph>::vertex_descriptor 
        mcr_vertex_t;
      typedef typename boost::graph_traits<TGraph>::edge_descriptor     
        mcr_edge_t;

      const TGraph&     m_g;
      typedef   std::vector<double>     eigenmode_t;
      eigenmode_t       m_eigen_value;
      eigenmode_t       m_eigen_vector;
      TVertexIndexMap   m_vim;
      TWeight1EdgeMap   m_ew1m;
      TWeight2EdgeMap   m_ew2m;

      typedef typename boost::remove_const<typename boost::property_traits<TWeight1EdgeMap>::value_type>::type mcr_edge_weight1_t;
      typedef typename boost::remove_const<typename boost::property_traits<TWeight2EdgeMap>::value_type>::type  mcr_edge_weight2_t;
      typedef typename boost::adjacency_list<
                         boost::listS, boost::vecS, boost::bidirectionalS, 
                         boost::no_property, 
                         boost::property<boost::edge_weight_t, 
                                         mcr_edge_weight1_t, 
                                         boost::property<boost::edge_weight2_t,
                                                         mcr_edge_weight2_t> > >
        pi_graph_t;
      typedef typename boost::property_map<pi_graph_t, boost::vertex_index_t>::type   TPiGraphVertexIndexMap;
      typedef typename boost::property_map<pi_graph_t, boost::edge_weight_t>::type    TPiGraphEdgeWeight1Map;
      typedef typename boost::property_map<pi_graph_t, boost::edge_weight2_t>::type   TPiGraphEdgeWeight2Map;
        
      typedef typename boost::property_traits<TPiGraphVertexIndexMap>::value_type     pigraph_vertex_index_t;
        
      pi_graph_t        m_pi_g;
      typedef   typename boost::graph_traits<pi_graph_t>::vertex_descriptor pi_vertex_t;
      typedef   typename boost::graph_traits<pi_graph_t>::edge_descriptor pi_edge_t;
      typedef   typename boost::iterator_property_map<typename std::vector<pi_vertex_t>::iterator, TVertexIndexMap> g2pi_g_vm_t;
      g2pi_g_vm_t m_g2pi_g_vm; ///Graph to Pi graph vertex map
      std::vector<pi_vertex_t> m_g2pig;
      int       m_step_number;
      const double m_minus_infinity;
      typedef typename std::vector<mcr_edge_t>        critical_cycle_t;
      double m_cr; ///Cycle ratio that already has been found

      class bad_graph 
      {
      public:
        typedef typename boost::property_traits<TVertexIndexMap>::value_type
          v_index_t;

        bad_graph(v_index_t bvi) : bad_vertex_index(bvi) {}
        v_index_t what() const throw() 
        {
          return bad_vertex_index;
        }

      private:
        v_index_t       bad_vertex_index;
      };

      double maximum_cycle_ratio_Howard()
      {
        try 
          {
            construct_pi_graph();
          }
        catch (const bad_graph& a)
          {
            return m_minus_infinity;
          }
        std::vector<double>  max_eigen_val(boost::num_vertices(m_g));
        m_eigen_value.resize(boost::num_vertices(m_g)); 
        m_eigen_vector.resize(boost::num_vertices(m_g));
        m_step_number = 0;
        do 
          {
            pi_eingen_value(get(vertex_index, m_pi_g), get(boost::edge_weight, m_pi_g), get(boost::edge_weight2, m_pi_g));
            ++m_step_number;
          } 
        while (improve_policy_try1(max_eigen_val) || improve_policy_try2(max_eigen_val));
        return *(std::max_element(m_eigen_value.begin(), m_eigen_value.end()));
      }

      /*! 
       *  Construct an arbitrary policy m_pi_g. 
       */       
      void      construct_pi_graph() 
      {
        m_g2pig.resize(boost::num_vertices(m_g));
        m_g2pi_g_vm = boost::make_iterator_property_map(m_g2pig.begin(), m_vim);
        BGL_FORALL_VERTICES_T(vd, m_g, TGraph)
          {
            m_g2pi_g_vm[vd] = boost::add_vertex(m_pi_g);
            store_pivertex(m_g2pi_g_vm[vd], vd);
          }
        BGL_FORALL_VERTICES_T(vd1, m_g, TGraph)
          {
            if (boost::out_edges(vd1, m_g).first == boost::out_edges(vd1, m_g).second) throw bad_graph(m_vim[vd1]);
            mcr_edge_t ed = *boost::out_edges(vd1, m_g).first;
            pi_edge_t pied = boost::add_edge(m_g2pi_g_vm[source(ed, m_g)], m_g2pi_g_vm[target(ed, m_g)], m_pi_g).first;
            boost::put(boost::edge_weight, m_pi_g, pied, m_ew1m[ed]);
            boost::put(boost::edge_weight2, m_pi_g, pied, m_ew2m[ed]);
          }
      }
        
      class bfs_eingmode_visitor : public boost::default_bfs_visitor 
      {
      public:
        bfs_eingmode_visitor(TPiGraphVertexIndexMap vi_m, TPiGraphEdgeWeight1Map w_m, TPiGraphEdgeWeight2Map& d_m,
                             eigenmode_t& e_val, eigenmode_t& e_vec, double ev) : m_index_map(vi_m), m_weight_map(w_m), m_delay_map(d_m), 
                                                                                  m_eig_value(&e_val), m_eig_vec(&e_vec), m_eigen_value(ev) { }
                
        template < typename Edge, typename  g_t>
        void examine_edge(Edge e, const g_t & g) const
        {
          typedef       typename boost::graph_traits<g_t>::vertex_descriptor Vertex;
          Vertex u = boost::target(e, g), v = boost::source(e, g);
          pigraph_vertex_index_t ind = m_index_map[u];
          (*m_eig_value)[ind] =  m_eigen_value;
          (*m_eig_vec)[ind] = m_weight_map[e] - m_eigen_value * m_delay_map[e] + (*m_eig_vec)[m_index_map[v]];
        }
      private:
        TPiGraphVertexIndexMap  m_index_map; 
        TPiGraphEdgeWeight1Map  m_weight_map;
        TPiGraphEdgeWeight2Map  m_delay_map;
        eigenmode_t*    m_eig_value;
        eigenmode_t*    m_eig_vec;
        double                  m_eigen_value;
      };

      /*!
       *  Find a vertex in the Pi Graph which belongs to cycle, just a DFV until back edge found
       */
      pi_vertex_t       find_good_source(const pi_vertex_t start_vertex) 
      {
        pi_vertex_t     good_vertex = start_vertex;
        typename std::set<pi_vertex_t>  s; 
        s.insert(start_vertex);
        do 
          {
            good_vertex = boost::target(*boost::out_edges(good_vertex, m_pi_g).first, m_pi_g);
          } 
        while (s.insert(good_vertex).second);
        return good_vertex;
      }
      virtual   void    store_pivertex(pi_vertex_t pivd, mcr_vertex_t vd) {}
      virtual void      store_critical_edge(pi_edge_t   ed, critical_cycle_t& cc) {}
      virtual void      store_critical_cycle(critical_cycle_t& cc) {}
        
      /*!
       * \param startV - vertex that belongs to a cycle in policy graph m_pi_g
       */
      double    calculate_eigen_value(pi_vertex_t       startV) 
      {
        std::pair<double, double>       accum_sums(0., 0.);
        pi_vertex_t vd = startV;
        critical_cycle_t        cc;
        do 
          {
            pi_edge_t tmp_ed = *(boost::out_edges(vd, m_pi_g).first);
            store_critical_edge(tmp_ed, cc);
            accum_sums.first += boost::get(boost::edge_weight, m_pi_g, tmp_ed); 
            accum_sums.second += boost::get(boost::edge_weight2, m_pi_g, tmp_ed);
            vd = boost::target(tmp_ed, m_pi_g);
          } 
        while (vd != startV);
        //assert((std::abs<double>(accum_sums.first) <= 0.00000001) && "Division by zerro!");
        double cr = accum_sums.first / accum_sums.second;
        if (cr > m_cr) 
          {
            m_cr = cr;
            store_critical_cycle(cc);
          }
        else
          {
                                                        
          }
        return  cr;
      }

      /*!
       * Value determination. Find a generalized eigenmode (n^{k+1}, x^{k+1}) of A^{Ï_{k+1}} of the pi graph (Algorithm IV.1).
       */
      void pi_eingen_value(
                           TPiGraphVertexIndexMap index_map,
                           TPiGraphEdgeWeight1Map weight_map, 
                           TPiGraphEdgeWeight2Map weigh2_map) 
      {
        using namespace boost;
        typedef std::vector<default_color_type> color_map_t;
        color_map_t     vcm(num_vertices(m_pi_g), white_color);//Vertex color map
        color_map_t::iterator   uv_itr = vcm.begin(); //Undiscovered vertex
        reverse_graph<pi_graph_t>       rev_g(m_pi_g); //For backward breadth visit

        while ((uv_itr = std::find_if(uv_itr, vcm.end(), 
                                      boost::bind(std::equal_to<default_color_type>(), boost::white_color, _1))) != vcm.end()) 
          ///While there are undiscovered vertices
          {
            pi_vertex_t gv = find_good_source(pi_vertex_t(uv_itr - vcm.begin()));
            pigraph_vertex_index_t      gv_ind = index_map[gv];
            m_eigen_value[gv_ind] = calculate_eigen_value(gv) ;
            bfs_eingmode_visitor        bfs_vis(index_map, weight_map, weigh2_map, m_eigen_value, m_eigen_vector, m_eigen_value[gv_ind]); 
            typename boost::queue<pi_vertex_t> Q;
            breadth_first_visit(rev_g, gv, Q, bfs_vis, make_iterator_property_map(vcm.begin(), index_map));
          }
      }

      void improve_policy(mcr_vertex_t vd, mcr_edge_t new_edge)
      {
        remove_edge(*(out_edges(m_g2pi_g_vm[vd], m_pi_g).first), m_pi_g);
        pi_edge_t ned = add_edge(m_g2pi_g_vm[vd], m_g2pi_g_vm[target(new_edge, m_g)], m_pi_g).first;
        put(edge_weight, m_pi_g, ned, m_ew1m[new_edge]);
        put(edge_weight2, m_pi_g, ned, m_ew2m[new_edge]);               
      }
      /*!
       * Policy Improvement. Improve the policy graph. The new policy graph has greater cycle ratio.
       * \return false if nothing can be improved.
       */
      bool  improve_policy_try1(std::vector<double>&  max_eing_vals) 
      {
        bool  improved = false;
        BGL_FORALL_VERTICES_T(vd, m_g, TGraph) 
          {
            double      max_ev = m_minus_infinity;/// Maximum eigen value for vertex
            mcr_edge_t  cr_ed;///Critical edge

            BGL_FORALL_OUTEDGES_T(vd, outed, m_g, TGraph) 
              {
                if (m_eigen_value[m_vim[target(outed, m_g)]] > max_ev) 
                  {
                    max_ev = m_eigen_value[m_vim[boost::target(outed, m_g)]];
                    cr_ed = outed;
                  }
              }
            if (max_ev > m_eigen_value[get(m_vim,vd)]) 
              {
                improve_policy(vd, cr_ed);
                improved = true;
              }
            max_eing_vals[get(m_vim,vd)] = max_ev;
          }
        return  improved;
      }

      /*!
       * \param max_eigen_values[u] = max_(for all adjacent vertices (u,v)) m_eigen_value[v]
       */
      bool improve_policy_try2(const std::vector<double>& max_eigen_values) 
      {
        bool  improved = false;
        BGL_FORALL_VERTICES_T(vd, m_g, TGraph) 
          {
            mcr_edge_t  impr_edge;
            double      max_val = m_minus_infinity; 
            BGL_FORALL_OUTEDGES_T(vd, outed, m_g, TGraph) 
              {
                ///If vertex vd is in the K(vd) set 
                if (max_eigen_values[get(m_vim, vd)] <= m_eigen_value[get(m_vim, target(outed, m_g))])  
                  {
                    double c_val = m_ew1m[outed] - m_ew2m[outed] * m_eigen_value[m_vim[boost::target(outed, m_g)]] + 
                      m_eigen_vector[m_vim[boost::target(outed, m_g)]];
                    if (c_val > max_val) 
                      {
                        max_val = c_val;
                        impr_edge = outed;
                      }
                  }
              }
            if ((max_val - m_eigen_vector[get(m_vim, vd)]) > mcr_howard_ltolerance) 
              ///If m_eigen_vector[vd] == max_val
              {
                improve_policy(vd, impr_edge);
                improved = true;
              }
          }
        return improved;
      }
    };///Cmcr_Howard

    /*!
     * \return maximum cycle ratio and one critical cycle.
     */
    template <typename TGraph, typename TVertexIndexMap, typename TWeight1EdgeMap, typename TWeight2EdgeMap>
    class Cmcr_Howard1  : public        Cmcr_Howard<TGraph, TVertexIndexMap, TWeight1EdgeMap, TWeight2EdgeMap>
    {
    public:
      typedef Cmcr_Howard<TGraph, TVertexIndexMap, TWeight1EdgeMap, TWeight2EdgeMap> inhr_t;  
      Cmcr_Howard1(const TGraph& g, TVertexIndexMap vim, TWeight1EdgeMap ewm, TWeight2EdgeMap ew2m) : inhr_t(g, vim, ewm, ew2m) 
      { 
        m_pi_g2g.resize(boost::num_vertices(g));
        m_pi_g2g_vm = boost::make_iterator_property_map(m_pi_g2g.begin(), boost::get(boost::vertex_index, this->m_pi_g));
      }

      void      get_critical_cycle(typename inhr_t::critical_cycle_t& cc) { return cc.swap(m_critical_cycle); }
    protected:
      void      store_pivertex(typename inhr_t::pi_vertex_t pivd, typename inhr_t::mcr_vertex_t vd) 
      {
        m_pi_g2g_vm[pivd] = vd;
      }
      void      store_critical_edge(typename inhr_t::pi_edge_t  ed, typename inhr_t::critical_cycle_t& cc)
      {
        typename inhr_t::pi_vertex_t s = boost::source(ed, this->m_pi_g);
        typename inhr_t::pi_vertex_t t = boost::target(ed, this->m_pi_g);
        assert(boost::edge(m_pi_g2g_vm[s], m_pi_g2g_vm[t], this->m_g).second);
        cc.push_back(boost::edge(m_pi_g2g_vm[s], m_pi_g2g_vm[t], this->m_g).first); ///Store corresponding edge of the m_g
      }
      void      store_critical_cycle(typename inhr_t::critical_cycle_t& cc) 
      {
        m_critical_cycle.swap(cc);
      }
    private:
      typename inhr_t::critical_cycle_t m_critical_cycle;
      typedef   typename boost::iterator_property_map<typename std::vector<typename inhr_t::mcr_vertex_t>::iterator, typename inhr_t::TPiGraphVertexIndexMap> pi_g2g_vm_t;
      pi_g2g_vm_t       m_pi_g2g_vm; ///Maps policy graph vertices to input graph vertices
      typename std::vector<typename inhr_t::mcr_vertex_t> m_pi_g2g;
    };

    /*!
     * Add sink vertex - this will make any graph good, the selfloop will have ratio equal to infinity
     * Properties must be "self increasing"
     */
    template <typename TGraph, typename TWeight1EdgeMap, typename TWeight2EdgeMap>
    typename boost::graph_traits<TGraph>::vertex_descriptor 
    make_graph_good(TGraph& g, TWeight1EdgeMap ewm, TWeight2EdgeMap ew2m, 
                    typename boost::property_traits<TWeight1EdgeMap>::value_type infinity)
    {
      typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
      typename boost::graph_traits<TGraph>::vertex_descriptor sink = boost::add_vertex(g);
        
      BGL_FORALL_VERTICES_T(vd, g, TGraph) 
        {
          Edge newed = boost::add_edge(vd, sink, g).first;
          boost::put(ewm, newed, 0);
          boost::put(ew2m, newed, 1);
        }
      Edge selfed = boost::edge(sink, sink, g).first;
      boost::put(ewm, selfed, infinity);
      return sink;
    }

    /*!
     * Construct from input graph g "safe" (suitable for maximum_cycle_ratio1() call) version - safeg 
     */
    template <typename TG, typename TIndVertexMap, typename TW1EdgeMap, typename TW2EdgeMap, typename TSafeG, typename SafeG2GEdgeMap>
    void construct_safe_graph(const TG& g, TIndVertexMap vim, TW1EdgeMap ew1m, TW2EdgeMap ew2m, TSafeG& safeg, SafeG2GEdgeMap& sg2gm)
    {
      assert(num_vertices(g) == num_vertices(safeg));
      typedef typename graph_traits<TSafeG>::edge_descriptor tmp_edge_t;
      typedef typename graph_traits<TG>::edge_descriptor edge_t;
      typename graph_traits<TG>::edge_iterator  ei, ei_end;

      for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) 
        {
          tmp_edge_t tmped = add_edge(vim[source(*ei, g)], vim[target(*ei, g)], safeg).first;
          sg2gm[tmped] = *ei;
          put(edge_weight, safeg, tmped, get(ew1m, *ei));
          put(edge_weight2, safeg, tmped, get(ew2m, *ei));
        }
    }

    template <typename TGraph, typename TVertexIndexMap, typename TWeight1EdgeMap, typename TWeight2EdgeMap>
    double      maximum_cycle_ratio_good_graph(const TGraph& g, TVertexIndexMap vim, TWeight1EdgeMap ewm, TWeight2EdgeMap ew2m,
                                               typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0) 
    {
      if (pcc == 0)  
        {
          return        detail::Cmcr_Howard<TGraph, TVertexIndexMap, TWeight1EdgeMap, TWeight2EdgeMap>(g, vim, ewm, ew2m)();
        } 
      else 
        {
          detail::Cmcr_Howard1<TGraph, TVertexIndexMap, TWeight1EdgeMap, TWeight2EdgeMap> obj(g, vim, ewm, ew2m);
          double maxcr = obj();
          obj.get_critical_cycle(*pcc);
          return        maxcr;
        }
    }

    template <typename TGraph, typename TVertexIndexMap, typename TWeight1EdgeMap, typename TWeight2EdgeMap, typename TEdgeIndexMap>
    double  minimum_cycle_ratio_good_graph(const TGraph& g, TVertexIndexMap vim, TWeight1EdgeMap ewm, 
                                           TWeight2EdgeMap ew2m, TEdgeIndexMap eim,
                                           typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0)
    {
      typedef   typename boost::remove_const<typename boost::property_traits<TWeight1EdgeMap>::value_type>::type weight_value_t;
      BOOST_STATIC_ASSERT(!is_integral<weight_value_t>::value || is_signed<weight_value_t>::value);
      typename  std::vector<weight_value_t> ne_w(boost::num_edges(g));
      BGL_FORALL_EDGES_T(ed, g, TGraph)  ne_w[boost::get(eim, ed)] = -ewm[ed];
      return -maximum_cycle_ratio_good_graph(g, vim, boost::make_iterator_property_map(ne_w.begin(), eim), ew2m, pcc);
    }

    /*!
     * \param g directed multigraph.
     * \param pcc - pointer to the critical edges list.
     * \param minus_infinity must be small enough to garanty that g has at least one cycle with greater ratio.
     * \return minus_infinity if there're no cycles in the graph
     */
    template <typename TGraph, typename TWeight1EdgeMap, typename TWeight2EdgeMap>
    double      maximum_cycle_ratio1(const TGraph& g, TWeight1EdgeMap ewm, TWeight2EdgeMap ew2m,
                                     typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0,
                                     typename boost::property_traits<TWeight1EdgeMap>::value_type minus_infinity = -(std::numeric_limits<int>::max)()) 
    {
      typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;
      boost::function_requires< boost::ReadWritePropertyMapConcept<TWeight1EdgeMap, Edge> >();
      boost::function_requires< boost::ReadWritePropertyMapConcept<TWeight2EdgeMap, Edge> >();
        
      TGraph& ncg = const_cast<TGraph&>(g);
      Vertex sink = detail::make_graph_good(ncg, ewm, ew2m, minus_infinity );

      double res = maximum_cycle_ratio_good_graph(ncg, boost::get(boost::vertex_index, g), ewm, ew2m, pcc);
      boost::clear_vertex(sink, ncg); boost::remove_vertex(sink, ncg);
      return    res;
    }

    /*!
     * Edge index MUST be in diapazon [0,..., num_edges(g)-1]
     * \return plus_infinity if g has no cycles.
     */
    template <typename TGraph, typename TWeight1EdgeMap, typename TWeight2EdgeMap, typename TEdgeIndexMap>
    double      minimum_cycle_ratio1(const TGraph& g, TWeight1EdgeMap ewm, TWeight2EdgeMap ew2m, TEdgeIndexMap eim, 
                                     typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0,
                                     typename boost::property_traits<TWeight1EdgeMap>::value_type plus_infinity = (std::numeric_limits<int>::max)()
                                     ) 
    {
      typedef typename boost::property_traits<TEdgeIndexMap>::value_type ei_t;
      typedef typename boost::graph_traits<TGraph>::vertex_descriptor Vertex;
      typedef typename boost::graph_traits<TGraph>::edge_descriptor Edge;

      boost::function_requires< boost::ReadWritePropertyMapConcept<TWeight1EdgeMap, Edge> >();
      boost::function_requires< boost::ReadWritePropertyMapConcept<TWeight2EdgeMap, Edge> >();
      boost::function_requires< boost::ReadWritePropertyMapConcept<TEdgeIndexMap, Edge> >();
        
      TGraph& ncg = const_cast<TGraph&>(g);

      ei_t      nei = ei_t(boost::num_edges(g));
      Vertex sink = detail::make_graph_good(ncg, ewm, ew2m, plus_infinity );
      ///Maintain edge index invariant
      BGL_FORALL_VERTICES_T(vd, ncg, TGraph) 
        {
          typename boost::graph_traits<TGraph>::edge_descriptor ed = boost::edge(vd, sink, ncg).first;
          boost::put(eim, ed, nei++);
        }
      double res = minimum_cycle_ratio_good_graph(ncg, boost::get(boost::vertex_index, ncg), ewm, ew2m, eim, pcc);
      boost::clear_vertex(sink, ncg); boost::remove_vertex(sink, ncg);
      return    res;
    }
    struct edge_less_than  
    {
      template <typename TEdgeDescriptor> bool operator()(const TEdgeDescriptor& x, const TEdgeDescriptor& y) const
      {
        return x.get_property() < y.get_property();
      }
    };
  }///namespace detail
  namespace 
  {
    template <typename TW1, typename TW2> struct safe_graph 
    {
      typedef typename boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property, 
                                             typename boost::property<boost::edge_weight_t, TW1, typename boost::property<boost::edge_weight2_t, TW2> > > type;
    };
  }

  /*!
   * Calculate the maximum cycle ratio (mcr) of the directed multigraph g.
   * \param g directed multigraph
   * \param pcc - If provided then a critical cycle will be written to corresponding vector.
   * \param minus_infinity small enough value to garanty that g has at least one cycle with greater ratio.
   * \return mcr or minus_infinity if g has no cycles.
   */
  template <typename TGraph, typename TVertexIndexMap, typename TW1EdgeMap, typename TW2EdgeMap>
  double        maximum_cycle_ratio(const TGraph& g, TVertexIndexMap vim, TW1EdgeMap ew1m, TW2EdgeMap ew2m,
                                    typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0,
                                    typename boost::property_traits<TW1EdgeMap>::value_type minus_infinity = 
                                    -(std::numeric_limits<int>::max)())
  {
    typedef     typename remove_const<typename property_traits<TW1EdgeMap>::value_type>::type w1_t;
    typedef     typename remove_const<typename property_traits<TW2EdgeMap>::value_type>::type w2_t;
    typedef     typename safe_graph<w1_t, w2_t>::type safe_graph_t;
    typedef typename graph_traits<safe_graph_t>::edge_descriptor tmp_edge_t;
    typedef typename graph_traits<TGraph>::edge_descriptor edge_t;
    typename std::map<tmp_edge_t, edge_t, detail::edge_less_than>       tmpg2g;
    std::vector<tmp_edge_t> cc;
    safe_graph_t sg(num_vertices(g));
    detail::construct_safe_graph(g, vim, ew1m, ew2m, sg, tmpg2g);
    double  mcr = maximum_cycle_ratio1(sg, get(edge_weight, sg), get(edge_weight2, sg), pcc ? &cc : 0, minus_infinity);
    if (pcc && (mcr > minus_infinity)) 
      {
        pcc->clear();
        for (typename std::vector<tmp_edge_t>::iterator it = cc.begin(); it != cc.end(); ++it) pcc->push_back(tmpg2g[*it]);
      }
    return mcr;
  }

  template <typename TGraph, typename TVertexIndexMap, typename TW1EdgeMap, typename TW2EdgeMap, typename TIndEdgeMap>
  double        minimum_cycle_ratio(const TGraph& g, TVertexIndexMap vim, TW1EdgeMap ew1m, TW2EdgeMap ew2m, TIndEdgeMap eim, 
                                    typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0,
                                    typename boost::property_traits<TW1EdgeMap>::value_type plus_infinity = 
                                    (std::numeric_limits<int>::max)())
  {
    typedef     typename boost::remove_const<typename boost::property_traits<TW1EdgeMap>::value_type>::type weight_value_t;
    BOOST_STATIC_ASSERT(!is_integral<weight_value_t>::value || is_signed<weight_value_t>::value);
    typename    std::vector<weight_value_t> ne_w(boost::num_edges(g));
    BGL_FORALL_EDGES_T(ed, g, TGraph)  ne_w[boost::get(eim, ed)] = -ew1m[ed];
    return -maximum_cycle_ratio(g, vim, boost::make_iterator_property_map(ne_w.begin(), eim), ew2m, pcc, -plus_infinity);
  }
  /*!
   * Calculate maximum mean cycle of directed weighted multigraph.
   * \param g directed multigraph
   * \return maximum mean cycle of g or minus_infinity if g has no cycles.
   */
  template <typename TGraph, typename TVertexIndexMap, typename TWeightEdgeMap, typename TIndEdgeMap>
  double  maximum_mean_cycle(const TGraph& g, TVertexIndexMap vim, TWeightEdgeMap ewm, TIndEdgeMap eim,
                             typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0,
                             typename boost::property_traits<TWeightEdgeMap>::value_type minus_infinity =
                             -(std::numeric_limits<int>::max)())
  {
    typedef     typename boost::remove_const<typename boost::property_traits<TWeightEdgeMap>::value_type>::type weight_value_t;
    typedef     typename boost::graph_traits<TGraph>::edge_descriptor Edge;
    typename    std::vector<weight_value_t> ed_w2(boost::num_edges(g), 1);
    return maximum_cycle_ratio(g, vim, ewm, boost::make_iterator_property_map(ed_w2.begin(), eim), pcc, minus_infinity);
  }

  template <typename TGraph, typename TVertexIndexMap, typename TWeightEdgeMap, typename TIndEdgeMap>
  double  minimum_mean_cycle(const TGraph& g, TVertexIndexMap vim, TWeightEdgeMap ewm, TIndEdgeMap eim,
                             typename std::vector<typename boost::graph_traits<TGraph>::edge_descriptor>* pcc = 0,
                             typename boost::property_traits<TWeightEdgeMap>::value_type plus_infinity =
                             (std::numeric_limits<int>::max)())
  {
    typedef     typename boost::remove_const<typename boost::property_traits<TWeightEdgeMap>::value_type>::type weight_value_t;
    typedef     typename boost::graph_traits<TGraph>::edge_descriptor Edge;
    typename    std::vector<weight_value_t> ed_w2(boost::num_edges(g), 1);
    return      minimum_cycle_ratio(g, vim, ewm, boost::make_iterator_property_map(ed_w2.begin(), eim), eim, pcc, plus_infinity);
  }
} //namespace boost
#endif
