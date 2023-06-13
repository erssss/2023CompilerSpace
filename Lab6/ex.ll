; ModuleID = 'ex.c'
source_filename = "ex.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @maxArea(i32* %0, i32 %1) #0 {
  %3 = alloca i32*, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32* %0, i32** %3, align 8
  store i32 %1, i32* %4, align 4
  store i32 0, i32* %5, align 4
  %9 = load i32, i32* %4, align 4
  %10 = sub nsw i32 %9, 1
  store i32 %10, i32* %6, align 4
  store i32 -1, i32* %7, align 4
  br label %11

11:                                               ; preds = %71, %2
  %12 = load i32, i32* %5, align 4
  %13 = load i32, i32* %6, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %72

15:                                               ; preds = %11
  %16 = load i32*, i32** %3, align 8
  %17 = load i32, i32* %5, align 4
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds i32, i32* %16, i64 %18
  %20 = load i32, i32* %19, align 4
  %21 = load i32*, i32** %3, align 8
  %22 = load i32, i32* %6, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds i32, i32* %21, i64 %23
  %25 = load i32, i32* %24, align 4
  %26 = icmp slt i32 %20, %25
  br i1 %26, label %27, label %37

27:                                               ; preds = %15
  %28 = load i32, i32* %6, align 4
  %29 = load i32, i32* %5, align 4
  %30 = sub nsw i32 %28, %29
  %31 = load i32*, i32** %3, align 8
  %32 = load i32, i32* %5, align 4
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds i32, i32* %31, i64 %33
  %35 = load i32, i32* %34, align 4
  %36 = mul nsw i32 %30, %35
  store i32 %36, i32* %8, align 4
  br label %47

37:                                               ; preds = %15
  %38 = load i32, i32* %6, align 4
  %39 = load i32, i32* %5, align 4
  %40 = sub nsw i32 %38, %39
  %41 = load i32*, i32** %3, align 8
  %42 = load i32, i32* %6, align 4
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds i32, i32* %41, i64 %43
  %45 = load i32, i32* %44, align 4
  %46 = mul nsw i32 %40, %45
  store i32 %46, i32* %8, align 4
  br label %47

47:                                               ; preds = %37, %27
  %48 = load i32, i32* %8, align 4
  %49 = load i32, i32* %7, align 4
  %50 = icmp sgt i32 %48, %49
  br i1 %50, label %51, label %53

51:                                               ; preds = %47
  %52 = load i32, i32* %8, align 4
  store i32 %52, i32* %7, align 4
  br label %53

53:                                               ; preds = %51, %47
  %54 = load i32*, i32** %3, align 8
  %55 = load i32, i32* %5, align 4
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds i32, i32* %54, i64 %56
  %58 = load i32, i32* %57, align 4
  %59 = load i32*, i32** %3, align 8
  %60 = load i32, i32* %6, align 4
  %61 = sext i32 %60 to i64
  %62 = getelementptr inbounds i32, i32* %59, i64 %61
  %63 = load i32, i32* %62, align 4
  %64 = icmp sgt i32 %58, %63
  br i1 %64, label %65, label %68

65:                                               ; preds = %53
  %66 = load i32, i32* %6, align 4
  %67 = sub nsw i32 %66, 1
  store i32 %67, i32* %6, align 4
  br label %71

68:                                               ; preds = %53
  %69 = load i32, i32* %5, align 4
  %70 = add nsw i32 %69, 1
  store i32 %70, i32* %5, align 4
  br label %71

71:                                               ; preds = %68, %65
  br label %11

72:                                               ; preds = %11
  %73 = load i32, i32* %7, align 4
  ret i32 %73
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca [10 x i32], align 16
  store i32 0, i32* %1, align 4
  %4 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 0
  store i32 3, i32* %4, align 16
  %5 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 1
  store i32 3, i32* %5, align 4
  %6 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 2
  store i32 9, i32* %6, align 8
  %7 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 3
  store i32 0, i32* %7, align 4
  %8 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 4
  store i32 0, i32* %8, align 16
  %9 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 5
  store i32 1, i32* %9, align 4
  %10 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 6
  store i32 1, i32* %10, align 8
  %11 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 7
  store i32 5, i32* %11, align 4
  %12 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 8
  store i32 7, i32* %12, align 16
  %13 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 9
  store i32 8, i32* %13, align 4
  store i32 10, i32* %2, align 4
  %14 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 0
  %15 = load i32, i32* %2, align 4
  %16 = call i32 @maxArea(i32* %14, i32 %15)
  store i32 %16, i32* %2, align 4
  %17 = load i32, i32* %2, align 4
  ret i32 %17
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
