((nil . ((projectile-project-compilation-cmd . "make build")
         (projectile-project-run-cmd . "make run")))
 (c-mode . ((dape-configs . ((gdb
                              command "gdb"
                              command-args ["-i" "dap"]
                              modes (c-mode)
                              command "gdb"
                              :program "lispy"
                              :args []
                              :cwd dape-cwd-fn
                              :type "gdb"
                              :request "launch"))))))
