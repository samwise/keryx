#pragma once

namespace keryx {
template <int N, typename Target, typename...> struct GetTypeIdxImp {};
template <int N, typename Target> struct GetTypeIdxImp<N, Target> {};
template <int N, typename Target, typename First, typename... Rest>
struct GetTypeIdxImp<N, Target, First, Rest...> {
   static constexpr int value =
       std::conditional<std::is_same<Target, First>::value,
                        std::integral_constant<int, N>,
                        GetTypeIdxImp<N + 1, Target, Rest...>>::type::value;
};
template <typename Target, typename... TypeList> struct GetTypeIdx {
   static constexpr int value = GetTypeIdxImp<0, Target, TypeList...>::value;
};
template <typename Target, class... Types> struct GetTypeIdxInTuple {};
template <typename Target, class... TypeList>
struct GetTypeIdxInTuple<Target, std::tuple<TypeList...>> {
   static constexpr int value = GetTypeIdxImp<0, Target, TypeList...>::value;
};

}
