require File.join( File.dirname(__FILE__), 'lib_finder.rb' )

module RestinioPCREFind

  def self.find_libs( libs )

    lib_dirs = ENV[ "LIB" ]
    if lib_dirs

      dirs_to_look = lib_dirs.split( ';' ).select{|p| p != "" }

      dir = dirs_to_look.find_index{|d| RestinioLibFinder.all_files_exist( d, libs ) }
    end

    if dir
      libs
    else
      []
    end
  end

  def self.get_libs_vc
    find_libs( [ 'pcre.lib' ] )
  end

  def self.get_libs_linux
    [ 'pcre' ]
  end

  def self.get_libs_names_linux
    [ 'libpcre.a' ]
  end

  def self.get_libs_mingw
    [ 'pcre' ]
  end

  def self.get_libs_names_mingw
    [ 'libpcre.a' ]
  end

  def self.get_libs( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_mingw
        end
    else
      get_libs_linux
    end
  end

  def self.get_libs_names( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_names_mingw
        end
    else
      get_libs_names_linux
    end
  end

  def self.try_to_get_custom_file
    RestinioLibFinder.try_to_get_custom_file( 'PCRE_PRJ_FILE', 'local-pcre.rb' )
  end


  def self.has_pcre( toolset )
    custom_prj = try_to_get_custom_file
    if File.exist?(custom_prj)
      true
    else
      libraries = get_libs_names( toolset )
      if !libraries.empty?
        RestinioLibFinder.check_libs_available( toolset, libraries )
      else
        false
      end
    end
  end


end #module RestinioPCREFind
