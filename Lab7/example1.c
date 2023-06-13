define i32 @ifElse () {
B10:
  %t11 = alloca i32, align 4
  store i32 5, i32* %t11, align 4
  %t2 = load i32, i32* %t11, align 4
  %t3 = icmp eq i32 %t2, 5
  %t15 = icmp ne i32 %t3, 0
  br i32 %t15, label %B12, label %B14
B12:                               	; preds = %B10
  store i32 25, i32* %t11, align 4
  br label %B13
B14:                               	; preds = %B10
  %t6 = load i32, i32* %t11, align 4
  %t7 = mul i32 %t6, 2
  store i32 %t7, i32* %t11, align 4
  br label %B13
B13:                               	; preds = %B12, %B14
  %t8 = load i32, i32* %t11, align 4
  ret i32 %t8
}
define i32 @main () {
B16:
  %t17 = call i32 @ifElse()
  ret i32 %t17
}
