package webRRRR

import (
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	v_cli "vk/cli"
)

func Start() {

	//	v_tmpls.SetTemplates()
	//	v_tmpls.SetMux()

	go startListen()
}

/*
func makeWrappedTmplHandler(tmplName string, flag_value string, match string) http.HandlerFunc {

	this_tmpl := ""

	fn := func(w http.ResponseWriter, r *http.Request) {
		this_tmpl = tmplName

		err := v_tmpls.Tmpls.ExecuteTemplate(w, this_tmpl, r)

		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
		}
	}

	return fn
}
*/

func startListen() {

	prefix := v_cli.Param(v_cli.CliFileServPrefix)
	f_root := v_cli.Param(v_cli.CliFileRoot)
	l_port := v_cli.Param(v_cli.CliWebPort)
	net_addr := v_cli.Param(v_cli.CliNetAddr)

	http.Handle(prefix, http.StripPrefix(prefix, http.FileServer(http.Dir(f_root))))

	listen_addr := ":" + l_port

	//	fmt.Printf("PREFIX: %s\nFILE ROOT: %s\nIP PORT: %s\nNET ADDR: %s\nLISTEN ADDR: %s\n",
	//		prefix, f_root, l_port, net_addr, listen_addr)

	//	fmt.Printf("PLESANS: %v\n", http.StripPrefix(prefix, http.FileServer(http.Dir(f_root))))

	fmt.Println("Listen...")

	if net_addr != "" {
		net_type := v_cli.Param(v_cli.CliNet)
		app_env := v_cli.Param(v_cli.CliAppEnv)
		addr_out := v_cli.Param(v_cli.CliAddrOut)
		l, err := net.Listen(net_type, net_addr)
		if err != nil {
			err = fmt.Errorf("Error! %s (env '%s')", err.Error(), app_env)

			panic(err)
		}

		tmp_str := l.Addr().Network() + ":" + l.Addr().String()

		err = ioutil.WriteFile(addr_out, []byte(tmp_str), 0644)
		if err != nil {
			err = fmt.Errorf("Error! %s (env '%s')", err.Error(), app_env)

			panic(err)
		}

		s := &http.Server{}

		//		fmt.Println("Volodja Dakšs...")

		panic(s.Serve(l))

	} else {
		panic(http.ListenAndServe(listen_addr, nil))
	}
}
