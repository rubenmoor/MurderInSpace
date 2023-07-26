check {
    name = "unreal-broken-array-call",
    description = [[
        Detects usages of array functions where the item parameter is a reference
        back into the array, and where the reference originates from a for-range
        loop.
        
        This detects bad code like this:

        for (const auto& Val : Arr)
        {
            Arr.Remove(Val);
        }

        which is invalid because the array will free the memory that Val is
        pointing to. Unreal checks this at runtime and will assert; this clang-tidy
        rule detects it at compile time.

        To fix this code, make the iteration value a copy of the value
        from the array, or index into the array instead of using a for-range loop:

        for (auto Val : Arr)
        {
            Arr.Remove(Val);
        }
    ]],
    matcher = [[
        cxxMemberCallExpr(
            on(
                declRefExpr(
                    hasType(cxxRecordDecl(hasName("TArray"))), 
                    to(decl().bind("array_declared_here"))
                ).bind("array_callsite")
            ), 
            callee(
                cxxMethodDecl(
                    matchesName("(Insert|Insert_GetRef|Add|Add_GetRef|Remove|RemoveSwap)")
                )
            ), 
            hasArgument(
                0, 
                declRefExpr(
                    to(
                        varDecl(
                            hasType(referenceType()), 
                            hasAncestor(
                                cxxForRangeStmt(
                                    hasRangeInit(
                                        declRefExpr(
                                            to(decl(equalsBoundNode("array_declared_here")))
                                        )
                                    )
                                ).bind("array_for_range")
                            )
                        ).bind("dangerous_ref_declaration")
                    )
                ).bind("dangerous_ref_usage")
            )
        ).bind("bad_callsite")
    ]],
    message = [[
        incorrect usage of mutating array call with non-copy will lead to crash at runtime
    ]],
    callsite = "bad_callsite",
    hints = {
        array_for_range = "make this a copy instead of a ref (i.e. not const&), or switch to an index-based for loop",
    }
}

check {
    name = "unreal-bad-reference-capture-in-delegate",
    description = [[
        Detects when you capture a reference as a user parameter to a delegate. This is almost always incorrect, as the delegate will capture the reference, not the value. When the referenced memory goes out of scope, the delegate can be invoked with arguments that point to an invalid memory space.

        For example:

        DECLARE_DELEGATE(FSomeFunctionHandler)
        void FunctionHandler(const int& UserParam) {};
        void BadImpl()
        {
            int A = 5;
            FSomeFunctionHandler::CreateStatic(&FunctionHandler, A);
            // If CreateStatic was passed to an event to be called after BadImpl
            // returns, it would be invoked with UserParam pointing to invalid memory.
        }

        You should always pass user parameters by value.
    ]],
    matcher = [[
        declRefExpr(to(functionDecl(hasName("(Create|Bind)(Lambda|Raw|SP|Static|ThreadSafeSP|UFunction|UObject|WeakLambda)"), isTemplateInstantiation(), hasAnyTemplateArgument(refersToPack(refersToType(lValueReferenceType().bind("captured_reference"))))))).bind("bad_callsite")
    ]],
    message = [[creating or binding of delegate incorrectly captures reference]],
    callsite = "bad_callsite",
    hints = {
        captured_reference = "this is the reference that is incorrectly captured. change the function declaration so this is passed by value instead."
    }
}

check {
    name = "unreal-missing-super-call-begindestroy",
    description = [[
        Detects when you forget to call Super::BeginDestroy(); in an overridden BeginDestroy function.
    ]],
    matcher = [[
        cxxMethodDecl(
            isOverride(),
            hasName("BeginDestroy"),
            hasBody(compoundStmt()),
            unless(
                hasDescendant(
                    cxxMemberCallExpr(
                        callee(cxxMethodDecl(hasName("BeginDestroy"))),
                        on(cxxThisExpr())
                    )
                )
            )
        ).bind("bad_impl")
    ]],
    message = [[missing call to Super::BeginDestroy(). make sure you call Super::BeginDestroy() in your overridden implementation.]],
    callsite = "bad_impl"
}

check {
    name = "unreal-ionlinesubsystem-get",
    description = [[
        Detects when you use IOnlineSubsystem::Get(). You should always use Online::GetSubsystem() instead.
    ]],
    matcher = [[
        callExpr(
            callee(
                cxxMethodDecl(
                    ofClass(
                        cxxRecordDecl(
                            hasName("IOnlineSubsystem")
                        )
                    ),
                    hasName("Get")
                )
            )
        ).bind("bad_callsite")
    ]],
    message = [[use Online::GetSubsystem(this->GetWorld()) instead of IOnlineSubsystem::Get(), as IOnlineSubsystem::Get() can't be safely used in the editor.]],
    callsite = "bad_callsite"
}

check {
    name = "unreal-unsafe-storage-of-oss-pointer",
    description = [[
        Detects when you try to store an IOnlineSubsystemPtr or IOnline*Ptr as a field in a class or struct.
        
        You can not safely store these values, as they will prevent the online subsystem from releasing it's resources, which it may do when the editor is open and play-in-editor is started or stopped.

        You should use weak pointers e.g. TWeakPtr<IOnlineIdentity, ESPMode::ThreadSafe> instead.
    ]],
    matcher = [[
        fieldDecl(
            hasType(
                hasUnqualifiedDesugaredType(
                    recordType(
                        hasDeclaration(
                            classTemplateSpecializationDecl(
                                hasName("TSharedPtr"),
                                hasTemplateArgument(
                                    0,
                                    refersToType(
                                        hasDeclaration(
                                            namedDecl(
                                                matchesName("IOnline[A-Za-z0-9]+")
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        ).bind("bad_field")
    ]],
    message = [[storing shared pointers of this type at the struct or class level is unsafe, as it will prevent the online subsystem from safely releasing it's resources. use a TWeakPtr<> instead.]],
    callsite = "bad_field"
} 

check {
    name = "unreal-missing-uproperty",
    description = [[
        Detects when you forget to add UPROPERTY() to fields in a UCLASS(), where those fields point to other garbage collected objects.
    ]],
    matcher = [[
        fieldDecl(
            unless(
                isUProperty()
            ), 
            hasParent(
                cxxRecordDecl(
                    isUClass()
                )
            ), 
            hasType(
                pointerType(
                    pointee(
                        recordType(
                            hasDeclaration(
                                cxxRecordDecl(
                                    isUClass()
                                )
                            )
                        )
                    )
                )
            )
        ).bind("bad_field")
    ]],
    message = "missing UPROPERTY() declaration on this field, which is required for all UObject pointers",
    callsite = "bad_field"
}

check {
    name = "unreal-missing-super-call-lifetimeprops",
    description = [[
        Detects when you forget to call Super::GetLifetimeReplicatedProps(); in an overridden GetLifetimeReplicatedProps function.
    ]],
    matcher = [[
        cxxMethodDecl(
            isOverride(),
            hasName("GetLifetimeReplicatedProps"),
            hasBody(compoundStmt()),
            ofClass(
                cxxRecordDecl(
                    isDirectlyDerivedFrom(
                        typedefNameDecl(
                            hasType(
                                type().bind("parent_class")
                            )
                        )
                    )
                )
            ),
            unless(
                hasDescendant(
                    cxxMemberCallExpr(
                        callee(
                            cxxMethodDecl(
                                hasName("GetLifetimeReplicatedProps")
                            )
                        ),
                        anyOf(
                            thisPointerType(
                                qualType(
                                    recordType(
                                        equalsBoundNode("parent_class")
                                    )
                                )
                            ),
                            onImplicitObjectArgument(
                                implicitCastExpr(
                                    has(
                                        implicitCastExpr(
                                            hasImplicitDestinationType(
                                                qualType(
                                                    pointsTo(
                                                        typedefType(
                                                            hasDeclaration(
                                                                typedefDecl(
                                                                    hasName("Super")
                                                                )
                                                            )
                                                        )
                                                    )
                                                )
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        ).bind("bad_impl")
    ]],
    message = [[missing call to Super::GetLifetimeReplicatedProps(). make sure you call Super::GetLifetimeReplicatedProps() in your overridden implementation.]],
    callsite = "bad_impl"
}

check {
    name = "unreal-missing-doreplifetime-for-replicated-property",
    description = [[
        Detects when you forget to call DOREPLIFETIME() for a replicated property.
    ]],
    matcher = [[
        cxxMethodDecl(
            hasName("GetLifetimeReplicatedProps"), 
            ofClass(
                cxxRecordDecl(
                    forEach(
                        fieldDecl(
                            isUProperty(), 
                            hasUSpecifier("replicated")
                        ).bind("declared_property")
                    )
                )
            ), 
            hasBody(
                compoundStmt(
                    forNoDescendant(
                        callExpr(
                            callee(
                                namedDecl(
                                    hasName("GetMemberNameCheckedJunk")
                                )
                            ), 
                            hasArgument(
                                0, 
                                memberExpr(
                                    member(
                                        fieldDecl(
                                            equalsBoundNode("declared_property")
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        ).bind("configuration_site")
    ]],
    message = [[missing call to DOREPLIFETIME() or equivalent macro in GetLifetimeReplicatedProps for a replicated property.]],
    callsite = "declared_property",
    hints = {
        configuration_site = "this implementation needs to call DOREPLIFETIME() or an similar macro to configure this property for replication"
    }
}

check {
    name = "unreal-incorrect-interface-invocation",
    description = [[
        Detects when you call an IInterface method directly with I->Test(); instead of using IInterface::Execute_Test(I);
    ]],
    matcher = [[
        cxxMemberCallExpr(
            thisPointerType(
                hasDeclaration(
                    cxxRecordDecl(
                        isIInterface()
                    )
                )
            ),
            callee(
                cxxMethodDecl(
                    isUFunction(),
                    anyOf(
                        hasUSpecifier("BlueprintImplementableEvent"),
                        hasUSpecifier("BlueprintNativeEvent")
                    )
                )
            )
        ).bind("bad_callsite")
    ]],
    message = [[incorrect call to UInterface member method. you must call methods via IInterface::Execute_Func(Val, Arg1, Arg2, ...) instead of Val->Func(Arg1, Arg2, ...).]],
    callsite = "bad_callsite"
}

check {
    name = "unreal-ustruct-field-not-initialized-in-class",
    description = [[
        Detects when a USTRUCT property does not have an in-class initializer. This causes a runtime warning as of Unreal Engine 5 and is marked to cause a runtime error in future, so this check catches the issue at compile time.
    ]],
    matcher = [[
        fieldDecl(
            hasType(qualType(isPODType())),
            unless(hasType(cxxRecordDecl(isUStruct()))),
            unless(hasInClassInitializer(expr())),
            hasDeclContext(cxxRecordDecl(isUStruct()))
        ).bind("bad_field")
    ]],
    message = [[
        missing default initializer for POD-typed field declaration in USTRUCT (expected a zeroing initializer like 'int A = 0;' or 'bool B = false;')
    ]],
    callsite = "bad_field"
}