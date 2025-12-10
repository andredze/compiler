CXX = g++

CXXFLAGS = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Wsign-conversion 			 \
		   -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations 		   		 \
		   -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported  		   		 \
		    -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral 		 			 \
		   -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op 		   		 \
		   -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith  		 \
		   -Winit-self -Wredundant-decls -Wshadow -Wsign-promo		   		 					 \
		   -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn		   		 \
		   -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default 	 \
		   -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast 		 \
		   -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing  \
		   -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation 	 \
		   -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer 	 \
		   -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -Wconversion

CXXFLAGS += -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

CXXFLAGS += -I include -I ../common/lang_ctx -I ../common/stack -I ../common/colors -I ../common/tree -I ../common

SOURCES = src/main.cpp 						\
          src/lexer.cpp 					\
          ../common/colors/colors.cpp		\
          ../common/lang_ctx/lang_ctx.cpp	\
          ../common/stack/stack.cpp			\
          ../common/tree/tree_commands.cpp	\
          ../common/tree/tree_dump.cpp		\
#           src/parser.cpp 					\
#           src/data_read.cpp 				\

ifdef DEBUG
CXXFLAGS += -D TREE_DEBUG
endif

OBJS = $(SOURCES:src/%.cpp=obj/%.o)

TARGET = run

$(shell mkdir -p obj obj/colors obj/tree obj/stack)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(CXXFLAGS)

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# clean:
# 	rm -rf $(OBJS) $(TARGET)

dlog:
	rm -rf log/*

.PHONY: all clean
